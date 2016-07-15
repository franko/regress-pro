
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
#include "error-messages.h"
#include "minsampling.h"


static void build_fit_engine_cache(struct fit_engine *f);

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
build_fit_engine_cache(struct fit_engine *f)
{
    size_t dmultipl = (f->run->system_kind == SYSTEM_REFLECTOMETER ? 1 : 2);
    int RI_fixed = fit_parameters_are_RI_fixed(f->parameters);
    size_t nb = f->stack->nb;
    int nblyr = nb - 2;

    build_stack_cache(&f->run->cache, f->stack, f->run->spectr, RI_fixed);

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

int
fit_engine_prepare(struct fit_engine *fit, struct spectrum *s)
{
    struct fit_config *cfg = fit->config;
    enum system_kind syskind = s->config.system;

    fit->run->system_kind = syskind;
    fit->run->spectr = spectra_copy(s);

    if(fit->config->spectr_range.active)
        spectr_cut_range(fit->run->spectr,
                         cfg->spectr_range.min, cfg->spectr_range.max);

    if(cfg->subsampling) {
        if(syskind == SYSTEM_ELLISS_AB || syskind == SYSTEM_ELLISS_PSIDEL) {
            elliss_sample_minimize(fit->run->spectr, 0.05);
        }
    }

    build_fit_engine_cache(fit);

    switch(syskind) {
    case SYSTEM_REFLECTOMETER:
        fit->run->mffun.f      = & refl_fit_f;
        fit->run->mffun.df     = & refl_fit_df;
        fit->run->mffun.fdf    = & refl_fit_fdf;
        fit->run->mffun.n      = spectra_points(fit->run->spectr);
        fit->run->mffun.p      = fit->parameters->number;
        fit->run->mffun.params = fit;
        break;
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
        fit->run->mffun.f      = & elliss_fit_f;
        fit->run->mffun.df     = & elliss_fit_df;
        fit->run->mffun.fdf    = & elliss_fit_fdf;
        fit->run->mffun.n      = 2 * spectra_points(fit->run->spectr);
        fit->run->mffun.p      = fit->parameters->number;
        fit->run->mffun.params = fit;
        break;
    default:
        return 1;
    }

    if(! cfg->threshold_given) {
        cfg->chisq_threshold = (syskind == SYSTEM_REFLECTOMETER ? 150 : 3000);
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
check_fit_parameters(struct stack *stack, struct fit_parameters *fps, str_ptr *error_msg)
{
    size_t j, nb_med = (size_t) stack->nb;

    assert(stack->nb >= 2);

    for(j = 0; j < nb_med; j++) {
        if(disp_integrity_check(stack->disp[j])) {
            *error_msg = new_error_message(RECIPE_CHECK, "corrupted material card");
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
                *error_msg = new_error_message(RECIPE_CHECK, "reference to thickness of layer %i", fp->layer_nb);
                return 1;
            }
            break;
        case PID_LAYER_N:
            if(fp->layer_nb >= nb_med) {
                *error_msg = new_error_message(RECIPE_CHECK, "Reference to parameter of material number %i", fp->layer_nb);
                return 1;
            }

            if(disp_check_fit_param(stack->disp[fp->layer_nb], fp)) {
                str_t pname;
                str_init(pname, 15);
                get_param_name(fp, pname);
                *error_msg = new_error_message(RECIPE_CHECK,
                                 "Parameter %s makes no sense for layer %i",
                                 CSTR(pname), fp->layer_nb);
                str_free(pname);
                return 1;
            }
            break;
        default:
            *error_msg = new_error_message(RECIPE_CHECK, "ill-formed fit parameter");
            return 1;
        }
    }

    return 0;
}

struct fit_parameters *
fit_engine_get_all_parameters(struct fit_engine *fit) {
    fit_param_t fp[1];

    struct fit_parameters *fps = fit_parameters_new();

    fp->id = PID_FIRSTMUL;
    fit_parameters_add(fps, fp);

    stack_get_all_parameters(fit->stack, fps);

    return fps;
}

double
fit_engine_get_parameter_value(const struct fit_engine *fit,
                               const fit_param_t *fp)
{
    if(fp->id == PID_FIRSTMUL) {
        return fit->extra->rmult;
    } else {
        return stack_get_parameter_value(fit->stack, fp);
    }
    return 0.0;
}

double
fit_engine_get_seed_value(const struct fit_engine *fit, const fit_param_t *fp, const seed_t *s)
{
    if (s->type == SEED_UNDEF) {
        return fit_engine_get_parameter_value(fit, fp);
    }
    return s->seed;
}

static double
compute_rsquare(const gsl_vector *ref, const gsl_vector *b)
{
    double ssq = 0;
    unsigned int i;
    for (i = 0; i < ref->size; i++) {
        ssq += gsl_vector_get(ref, i) * gsl_vector_get(ref, i);
    }

    double rsq = 0;
    for (i = 0; i < ref->size; i++) {
        double diff = gsl_vector_get(b, i) - gsl_vector_get(ref, i);
        rsq += diff * diff;
    }
    return rsq / ssq;
}

double
fit_engine_estimate_param_grid_step(struct fit_engine *fit, const gsl_vector *x, const fit_param_t *fp, double delta)
{
    int fp_index = fit_parameters_find(fit->parameters, fp);

    gsl_vector *y0 = gsl_vector_alloc(fit->run->mffun.n);
    gsl_vector *y1 = gsl_vector_alloc(fit->run->mffun.n);
    gsl_vector *xtest = gsl_vector_alloc(fit->run->mffun.p);
    gsl_matrix *jacob = gsl_matrix_alloc(fit->run->mffun.n, fit->run->mffun.p);

    gsl_vector_view jview = gsl_matrix_column(jacob, fp_index);
    fit->run->mffun.df(x, fit, jacob);

    gsl_vector_memcpy(xtest, x);
    fit->run->mffun.f(xtest, fit, y0);

    while (1) {
        gsl_vector_set(xtest, fp_index, gsl_vector_get(x, fp_index) + delta);
        fit->run->mffun.f(xtest, fit, y1);

        gsl_vector_sub(y1, y0);
        gsl_vector_scale(y1, 1.0 / delta);

        double r2 = compute_rsquare(&jview.vector, y1);
        if (r2 < 0.2) break;

        delta /= 2;
    }

    gsl_vector_free(y0);
    gsl_vector_free(y1);
    gsl_vector_free(xtest);
    gsl_matrix_free(jacob);
    return delta;
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
fit_engine_new()
{
    struct fit_engine *fit = emalloc(sizeof(struct fit_engine));
    set_default_extra_param(fit->extra);
    fit->parameters = NULL;
    fit->stack = NULL;
    return fit;
}

void
fit_engine_bind(struct fit_engine *fit, const stack_t *stack, const struct fit_config *config, struct fit_parameters *parameters)
{
    fit->config[0] = *config;
    /* fit is not the owner of the "parameters", we just keep a reference */
    fit->parameters = parameters;
    fit->stack = stack_copy(stack);
}

void
fit_engine_bind_stack(struct fit_engine *fit, stack_t *stack)
{
    if (fit->stack) {
        stack_free(fit->stack);
    }
    fit->stack = stack;
}

stack_t *
fit_engine_yield_stack(struct fit_engine *fit)
{
    stack_t *s = fit->stack;
    fit->stack = NULL;
    return s;
}

void
fit_engine_free(struct fit_engine *fit)
{
    if (fit->stack) {
        stack_free(fit->stack);
    }
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
    str_init(pname, 15);
    str_init(value, 15);

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

void
fit_config_set_default(struct fit_config *cfg)
{
    cfg->threshold_given = 0;
    cfg->nb_max_iters = 30;
    cfg->subsampling = 1;
    cfg->spectr_range.active = 0;
    cfg->epsabs = 1.0E-7;
    cfg->epsrel = 1.0E-7;
}

int
fit_config_write(writer_t *w, const struct fit_config *config)
{
    writer_printf(w, "fit-config");
    writer_newline_enter(w);
    if (config->threshold_given) {
        writer_printf(w, "threshold %g", config->chisq_threshold);
        writer_newline(w);
    }
    writer_printf(w, "max-iterations %d", config->nb_max_iters);
    writer_newline(w);

    writer_printf(w, "subsampling %d", config->subsampling);
    writer_newline(w);

    if (config->spectr_range.active) {
        writer_printf(w, "wavelength-range %g %g", config->spectr_range.min, config->spectr_range.max);
        writer_newline(w);
    }

    writer_printf(w, "epsilon %g %g", config->epsabs, config->epsrel);
    writer_newline_exit(w);
    return 1;
}

int
fit_config_read(lexer_t *l, struct fit_config *config)
{
    if (lexer_check_ident(l, "fit-config")) goto config_exit;
    if (lexer_ident(l)) goto config_exit;
    if (strcmp(CSTR(l->store), "threshold") == 0) {
        config->threshold_given = 1;
        if (lexer_number(l, &config->chisq_threshold)) goto config_exit;
        if (lexer_ident(l)) goto config_exit;
    } else {
        config->threshold_given = 0;
    }
    if (strcmp(CSTR(l->store), "max-iterations")) goto config_exit;
    if (lexer_integer(l, &config->nb_max_iters)) goto config_exit;
    if (lexer_check_ident(l, "subsampling")) goto config_exit;
    if (lexer_integer(l, &config->subsampling)) goto config_exit;
    if (lexer_ident(l)) goto config_exit;
    if (strcmp(CSTR(l->store), "wavelength-range") == 0) {
        double lmin, lmax;
        if (lexer_number(l, &lmin)) goto config_exit;
        if (lexer_number(l, &lmax)) goto config_exit;
        config->spectr_range.active = 1;
        config->spectr_range.min = lmin;
        config->spectr_range.max = lmax;
        if (lexer_ident(l)) goto config_exit;
    } else {
        config->spectr_range.active = 0;
    }
    if (strcmp(CSTR(l->store), "epsilon")) goto config_exit;
    if (lexer_number(l, &config->epsabs)) goto config_exit;
    if (lexer_number(l, &config->epsrel)) goto config_exit;
    return 0;
config_exit:
    return 1;
}
