
/* fit-engine.c
 *
 * Copyright (C) 2005-2011 Francesco Abbate
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <assert.h>
#include <string.h>
#include "fit-engine.h"
#include "refl-fit.h"
#include "refl-kernel.h"
#include "elliss-fit.h"
#include "elliss.h"
#include "symtab.h"
#include "error-messages.h"
#include "minsampling.h"

enum {
    DISABLE_RI_PRECALC = 0,
    ENABLE_RI_PRECALC = 1,
};

static void build_fit_engine_cache(struct fit_engine *f, int allow_RI_precalc);

static void dispose_fit_engine_cache(struct fit_run *run);


void
build_stack_cache(struct stack_cache *cache, stack_t *stack,
                  struct spectrum *spectr, int th_only_optimize)
{
    size_t nb_med = stack->nb;
    size_t j;

    cache->nb_med = nb_med;
    cache->ns  = emalloc(nb_med * sizeof(cmpl));

    cache->deriv_info = emalloc(nb_med * sizeof(struct deriv_info));

    for(j = 0; j < nb_med; j++) {
        struct deriv_info *di = cache->deriv_info + j;
        size_t tpnb = disp_get_number_of_params(stack->disp[j]);

        di->is_valid = 0;
        di->val = (tpnb == 0 ? NULL : cmpl_vector_alloc(tpnb));
    }

    cache->th_only = th_only_optimize;

    if(th_only_optimize) {
        int k, npt = spectra_points(spectr);
        cmpl *ns;

        cache->ns_full_spectr = emalloc(nb_med * npt * sizeof(cmpl));

        for(k = 0, ns = cache->ns_full_spectr; k < npt; ns += nb_med, k++) {
            double lambda = get_lambda_by_index(spectr, k);
            stack_get_ns_list(stack, ns, lambda);
        }
    } else {
        cache->ns_full_spectr = NULL;
    }

    cache->is_valid = 1;
}

void
dispose_stack_cache(struct stack_cache *cache)
{
    int j, nb_med = cache->nb_med;

    if(! cache->is_valid) {
        return;
    }

    free(cache->ns);

    for(j = 0; j < nb_med; j++) {
        struct deriv_info *di = & cache->deriv_info[j];
        if(di->val) {
            cmpl_vector_free(di->val);
        }
    }
    free(cache->deriv_info);

    if(cache->ns_full_spectr) {
        free(cache->ns_full_spectr);
    }

    cache->is_valid = 0;
}

void
build_fit_engine_cache(struct fit_engine *f, int allow_RI_precalc)
{
    size_t dmultipl = (f->run->system_kind == SYSTEM_REFLECTOMETER ? 1 : 2);
    int RI_fixed = fit_parameters_are_RI_fixed(f->parameters);
    size_t nb = f->stack->nb;
    int nblyr = nb - 2;

    build_stack_cache(&f->run->cache, f->stack, f->run->spectr, allow_RI_precalc && RI_fixed);

    f->run->jac_th = gsl_vector_alloc(dmultipl * nblyr);

    switch(f->run->system_kind) {
    case SYSTEM_REFLECTOMETER:
        f->run->jac_n.refl = gsl_vector_alloc(2 * nb);
        break;
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
        f->run->jac_n.ell = cmpl_vector_alloc(2 * nb);
    default:
        /* */
        ;
    }
}

void
dispose_fit_engine_cache(struct fit_run *run)
{
    gsl_vector_free(run->jac_th);

    switch(run->system_kind) {
    case SYSTEM_REFLECTOMETER:
        gsl_vector_free(run->jac_n.refl);
        break;
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
        cmpl_vector_free(run->jac_n.ell);
    default:
        /* */
        ;
    }

    dispose_stack_cache(&run->cache);
}

int
fit_engine_apply_param(struct fit_engine *fit, const fit_param_t *fp,
                       double val)
{
    int res = 0;

    switch(fp->id) {
    case PID_FIRSTMUL:
        fit->extra->rmult = val;
        break;
    default:
        res = stack_apply_param(fit->stack, fp, val);
    }

    return res;
}

void
fit_engine_apply_parameters(struct fit_engine *fit,
                            const struct fit_parameters *fps,
                            const gsl_vector *x)
{
    size_t j;

    for(j = 0; j < fps->number; j++) {
        const fit_param_t *fp = fps->values + j;
        double pval = gsl_vector_get(x, j);
        int status;
        status = fit_engine_apply_param(fit, fp, pval);
        /* No error should never occurs here because the fit parameters
        are checked in advance. */
        assert(status == 0);
    }
}

void
fit_engine_commit_parameters(struct fit_engine *fit, const gsl_vector *x)
{
    struct fit_parameters const * fps = fit->parameters;
    fit_engine_apply_parameters(fit, fps, x);
}

void
fit_engine_prepare_spectrum(const struct fit_engine *fit, struct spectrum *s)
{
    const struct fit_config *c = fit->config;
    if(c->spectr_range.active) {
        double inf = c->spectr_range.min;
        double sup = c->spectr_range.max;
        spectr_cut_range(s, inf, sup);
    }

    const enum system_kind syskind = s->config.system;
    if(c->subsampling) {
        if(syskind == SYSTEM_ELLISS_AB || syskind == SYSTEM_ELLISS_PSIDEL) {
            elliss_sample_minimize(s, 0.05);
        }
    }
}

static int
setup_gsl_linfit_function(enum system_kind syskind, struct fit_engine *fit, const struct spectrum *s)
{
    switch(syskind) {
    case SYSTEM_REFLECTOMETER:
        fit->run->mffun.f      = & refl_fit_f;
        fit->run->mffun.df     = & refl_fit_df;
        fit->run->mffun.fdf    = & refl_fit_fdf;
        fit->run->mffun.n      = spectra_points(s);
        fit->run->mffun.p      = fit->parameters->number;
        fit->run->mffun.params = fit;
        break;
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
        fit->run->mffun.f      = & elliss_fit_f;
        fit->run->mffun.df     = & elliss_fit_df;
        fit->run->mffun.fdf    = & elliss_fit_fdf;
        fit->run->mffun.n      = 2 * spectra_points(s);
        fit->run->mffun.p      = fit->parameters->number;
        fit->run->mffun.params = fit;
        break;
    default:
        return 1;
    }
    return 0;
}

enum system_kind
fit_engine_batch_prepare(struct fit_engine *fit, struct spectrum *s)
{
    struct fit_config *cfg = fit->config;
    enum system_kind syskind = s->config.system;

    fit->run->system_kind = syskind;
    fit->run->spectr = NULL;

    build_fit_engine_cache(fit, DISABLE_RI_PRECALC);
    setup_gsl_linfit_function(syskind, fit, s);

    if(! cfg->thresold_given) {
        cfg->chisq_thresold = (syskind == SYSTEM_REFLECTOMETER ? 150 : 3000);
    }

    fit->run->results = gsl_vector_alloc(fit->parameters->number);

    return syskind;
}

void
fit_engine_attach_spectrum(struct fit_engine *fit, struct spectrum *s)
{
    fit->run->spectr = s;
    const int mult = (fit->run->system_kind == SYSTEM_REFLECTOMETER ? 1 : 2);
    fit->run->mffun.n = mult * spectra_points(s);
}

int
fit_engine_prepare(struct fit_engine *fit, struct spectrum *s)
{
    struct fit_config *cfg = fit->config;
    enum system_kind syskind = s->config.system;

    fit->run->system_kind = syskind;
    fit->run->spectr = spectra_copy(s);

    fit_engine_prepare_spectrum(fit, fit->run->spectr);
    build_fit_engine_cache(fit, ENABLE_RI_PRECALC);
    setup_gsl_linfit_function(syskind, fit, fit->run->spectr);

    if(! cfg->thresold_given) {
        cfg->chisq_thresold = (syskind == SYSTEM_REFLECTOMETER ? 150 : 3000);
    }

    fit->run->results = gsl_vector_alloc(fit->parameters->number);

#ifdef DEBUG_REGRESS
    if(syskind != SYSTEM_REFLECTOMETER) {
        elliss_fit_test_deriv(fit);
    }
#endif

    return 0;
}

void
fit_engine_disable(struct fit_engine *fit)
{
    dispose_fit_engine_cache(fit->run);
    spectra_free(fit->run->spectr);
    gsl_vector_free(fit->run->results);
}

int
check_fit_parameters(struct stack *stack, struct fit_parameters *fps)
{
    size_t j, nb_med = (size_t) stack->nb;

    assert(stack->nb >= 2);

    for(j = 0; j < nb_med; j++) {
        if(disp_integrity_check(stack->disp[j])) {
            notify_error_msg(INVALID_STRATEGY, "corrupted material card");
            return 1;
        }
    }

    for(j = 0; j < fps->number; j++) {
        fit_param_t *fp = fps->values + j;
        switch(fp->id) {
        case PID_FIRSTMUL:
            break;
        case PID_THICKNESS:
            if(fp->layer_nb == 0 || fp->layer_nb >= nb_med - 1) {
                notify_error_msg(INVALID_STRATEGY,
                                 "reference to thickness of layer %i",
                                 fp->layer_nb);
                return 1;
            }
            break;
        case PID_LAYER_N:
            if(fp->layer_nb >= nb_med) {
                notify_error_msg(INVALID_STRATEGY,
                                 "Reference to parameter of material number %i",
                                 fp->layer_nb);
                return 1;
            }

            if(disp_check_fit_param(stack->disp[fp->layer_nb], fp)) {
                str_t pname;
                str_init(pname, 16);
                get_param_name(fp, pname);
                notify_error_msg(INVALID_STRATEGY,
                                 "Parameter %s makes no sense for layer %i",
                                 CSTR(pname), fp->layer_nb);
                str_free(pname);
                return 1;
            }
            break;
        default:
            notify_error_msg(INVALID_STRATEGY, "ill-formed fit parameter");
            return 1;
        }
    }

    return 0;
}

struct fit_parameters *
fit_engine_get_all_parameters(struct fit_engine *fit) {
    struct stack *stack = fit->stack;
    int n_layers = stack->nb - 2;
    int n_params = 1 + n_layers;
    struct fit_parameters *fps;
    fit_param_t fp[1];
    int j;

    for(j = 1; j < n_layers + 2; j++) {
        disp_t *d = stack->disp[j];
        int np = disp_get_number_of_params(d);
        n_params += np;
    }

    fps = fit_parameters_new();

    fp->id = PID_FIRSTMUL;
    fit_parameters_add(fps, fp);

    fp->id = PID_THICKNESS;
    for(j = 1; j < n_layers + 1; j++) {
        fp->layer_nb = j;
        fit_parameters_add(fps, fp);
    }

    fp->id = PID_LAYER_N;
    for(j = 1; j < n_layers + 2; j++) {
        disp_t *d = stack->disp[j];
        int k, np = disp_get_number_of_params(d);
        fp->layer_nb = j;
        fp->model_id = disp_get_model_id(d);
        for(k = 0; k < np; k++) {
            fp->param_nb = k;
            fit_parameters_add(fps, fp);
        }
    }

    return fps;
}

double
fit_engine_get_parameter_value(const struct fit_engine *fit,
                               const fit_param_t *fp)
{
    if(fp->id == PID_FIRSTMUL) {
        return fit->extra->rmult;
    } else if(fp->id == PID_THICKNESS) {
        const struct stack *st = fit->stack;
        int layer_nb = fp->layer_nb;
        assert(layer_nb > 0 && layer_nb < st->nb - 1);
        return st->thickness[layer_nb-1];
    } else if(fp->id == PID_LAYER_N) {
        const struct stack *st = fit->stack;
        int layer_nb = fp->layer_nb;
        const disp_t *d = st->disp[layer_nb];
        assert(layer_nb > 0 && layer_nb <= st->nb - 1);
        return disp_get_param_value(d, fp);
    }

    assert(0);
    return 0.0;
}

void
fit_engine_generate_spectrum(struct fit_engine *fit, struct spectrum *ref,
                             struct spectrum *synth)
{
    enum system_kind syskind = ref->config.system;
    size_t nb_med = fit->stack->nb;
    struct data_table *table = synth->table[0].table;
    int j, npt = spectra_points(ref);
    cmpl *ns = emalloc(sizeof(cmpl) * nb_med);
    double const * ths;

    assert(spectra_points(ref) == spectra_points(synth));

    synth->config = ref->config;

    ths = stack_get_ths_list(fit->stack);

    for(j = 0; j < npt; j++) {
        double lambda = get_lambda_by_index(ref, j);

        data_table_set(table, j, 0, lambda);

        stack_get_ns_list(fit->stack, ns, lambda);

        switch(syskind) {
        case SYSTEM_REFLECTOMETER: {
            double r_raw = mult_layer_refl_ni(nb_med, ns, ths, lambda,
                                              NULL, NULL);
            data_table_set(table, j, 1, fit->extra->rmult * r_raw);
            break;
        }
        case SYSTEM_ELLISS_AB:
        case SYSTEM_ELLISS_PSIDEL: {
            const enum se_type se_type = GET_SE_TYPE(syskind);
            double phi0 = ref->config.aoi;
            double anlz = ref->config.analyzer;
            ell_ab_t ell;

            mult_layer_se_jacob(se_type, nb_med, ns, phi0,
                                ths, lambda, anlz, ell, NULL, NULL);

            data_table_set(table, j, 1, ell->alpha);
            data_table_set(table, j, 2, ell->beta);
            break;
        }
        default:
            /* */
            ;
        }
    }

    free(ns);
}

struct fit_engine *
build_fit_engine(struct symtab *symtab, struct seeds **seeds) {
    stack_t *stack;
    struct strategy *strategy;
    struct fit_engine *fit;

    stack    = retrieve_parsed_object(symtab, TL_TYPE_STACK,
                                      symtab->directives->stack);

    strategy = retrieve_parsed_object(symtab, TL_TYPE_STRATEGY,
                                      symtab->directives->strategy);

    if(stack == NULL || strategy == NULL) {
        return NULL;
    }

    if(check_fit_parameters(stack, strategy->parameters) != 0) {
        return NULL;
    }

    *seeds = strategy->seeds;

    fit = emalloc(sizeof(struct fit_engine));

    set_default_extra_param(fit->extra);

    fit->config[0] = symtab->config_table[0];

    /* fit is not the owner of the "parameters", we just keep a reference */
    fit->parameters = strategy->parameters;

    fit->stack = stack_copy(stack);

    return fit;
}

void
fit_engine_free(struct fit_engine *fit)
{
    stack_free(fit->stack);
    free(fit);
}

void
set_default_extra_param(struct extra_params *extra)
{
    extra->rmult = 1.0;
}

void
fit_engine_print_fit_results(struct fit_engine *fit, str_t text, int tabular)
{
    str_t pname, value;
    size_t j;

    str_set_null(text);
    str_init(pname, 16);
    str_init(value, 16);

    for(j = 0; j < fit->parameters->number; j++) {
        fit_param_t *fp = fit->parameters->values + j;
        get_param_name(fp, pname);
        if(tabular) {
            str_printf(value, "%.6g", gsl_vector_get(fit->run->results, j));
            str_pad(value, 12, ' ');
            str_append(text, value, 0);
        } else
            str_printf_add(text, "%9s : %.6g\n", CSTR(pname),
                           gsl_vector_get(fit->run->results, j));
    }

    str_free(value);
    str_free(pname);
}
