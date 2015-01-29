
/*
  $Id$
 */

#include <assert.h>
#include <string.h>

#include "defs.h"
#include "fit-engine.h"
#include "refl-fit.h"
#include "error-messages.h"
#include "elliss-multifit.h"
#include "refl-multifit.h"
#include "multi-fit-engine.h"
#include "symtab.h"


static int  mengine_apply_param_common(struct multi_fit_engine *fit,
                                       const fit_param_t *fp,
                                       double val);

static int  mengine_apply_param_priv(struct multi_fit_engine *fit,
                                     const fit_param_t *fp,
                                     double val, int sample);

static void build_multi_fit_engine_cache(struct multi_fit_engine *f);

static void dispose_multi_fit_engine_cache(struct multi_fit_engine *f);

void
build_multi_fit_engine_cache(struct multi_fit_engine *f)
{
    const int RI_IS_VARIABLE = 0;
    int nbmed = f->stack_list[0]->nb;
    int nblyr = nbmed - 2;
    size_t dmultipl = (f->system_kind == SYSTEM_REFLECTOMETER ? 1 : 2);

    /* We have just one cache for the fit engine.
       A cache for each sample is not needed because we assume that
       the RI are not fixed and so we don't do presampling of n values */
    build_stack_cache(& f->cache, f->stack_list[0],
                      f->spectra_list[0], RI_IS_VARIABLE);

    f->jac_th = gsl_vector_alloc(dmultipl * nblyr);

    switch(f->system_kind) {
    case SYSTEM_REFLECTOMETER:
        f->jac_n.refl = gsl_vector_alloc(2 * nbmed);
        break;
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
        f->jac_n.ell = cmpl_vector_alloc(2 * nbmed);
    default:
        /* */
        ;
    }
}

int
multi_fit_engine_prepare(struct multi_fit_engine *fit)
{
    struct fit_parameters const * common = fit->common_parameters;
    struct fit_parameters const * priv   = fit->private_parameters;
    struct fit_config *cfg = & fit->config;
    size_t nb_total_params;

    assert(fit->spectra_list != NULL);

    nb_total_params = common->number + fit->samples_number * priv->number;

    /* We suppose that all the spectra are of the same kind, so we just
       look the first one */
    fit->system_kind = fit->spectra_list[0]->config.system;

    if(fit->config.spectr_range.active) {
        int k;
        for(k = 0; k < fit->samples_number; k++)
            spectr_cut_range(fit->spectra_list[k],
                             fit->config.spectr_range.min,
                             fit->config.spectr_range.max);
    }

    build_multi_fit_engine_cache(fit);

    switch(fit->system_kind) {
        int k, npt;
    case SYSTEM_REFLECTOMETER:

        for(npt = 0, k = 0; k < fit->samples_number; k++) {
            npt += spectra_points(fit->spectra_list[k]);
        }

        fit->mffun.f      = & refl_multifit_f;
        fit->mffun.df     = & refl_multifit_df;
        fit->mffun.fdf    = & refl_multifit_fdf;
        fit->mffun.n      = npt;
        fit->mffun.p      = nb_total_params;
        fit->mffun.params = fit;
        break;
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
        for(npt = 0, k = 0; k < fit->samples_number; k++) {
            npt += 2 * spectra_points(fit->spectra_list[k]);
        }

        fit->mffun.f      = & elliss_multifit_f;
        fit->mffun.df     = & elliss_multifit_df;
        fit->mffun.fdf    = & elliss_multifit_fdf;
        fit->mffun.n      = npt;
        fit->mffun.p      = nb_total_params;
        fit->mffun.params = fit;
        break;
    default:
        return 1;
    }

    if(! cfg->threshold_given) {
        cfg->chisq_threshold = (fit->system_kind == SYSTEM_REFLECTOMETER ? 5.0E3 : 2.0E5);
    }

    cfg->chisq_threshold *= fit->samples_number;

    fit->results = gsl_vector_alloc(nb_total_params);
    fit->chisq   = gsl_vector_alloc(fit->samples_number);

    fit->initialized = 1;

    return 0;
}

void
dispose_multi_fit_engine_cache(struct multi_fit_engine *f)
{
    gsl_vector_free(f->jac_th);

    switch(f->system_kind) {
    case SYSTEM_REFLECTOMETER:
        gsl_vector_free(f->jac_n.refl);
        f->jac_n.refl = NULL;
        break;
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
        cmpl_vector_free(f->jac_n.ell);
        f->jac_n.ell = NULL;
    default:
        /* */
        ;
    }

    f->jac_th = NULL;

    dispose_stack_cache(& f->cache);
}

void
multi_fit_engine_disable(struct multi_fit_engine *fit)
{
    dispose_multi_fit_engine_cache(fit);

    gsl_vector_free(fit->results);
    fit->results = NULL;

    gsl_vector_free(fit->chisq);
    fit->chisq = NULL;

    fit->initialized = 0;
}

int
mengine_apply_param_common(struct multi_fit_engine *fit,
                           const fit_param_t *fp,
                           double val)
{
    int res = 0;

    switch(fp->id) {
        int j;
    case PID_FIRSTMUL:
        fit->extra.rmult = val;
        break;
    default:
        for(j = 0; j < fit->samples_number; j++) {
            res = stack_apply_param(fit->stack_list[j], fp, val);
            if(res) {
                break;
            }
        }
    }

    return res;
}

int
mengine_apply_param_priv(struct multi_fit_engine *fit,
                         const fit_param_t *fp,
                         double val, int sample)
{
    int res = 0;

    switch(fp->id) {
    case PID_FIRSTMUL:
        res = 1;
        break;
    default:
        res = stack_apply_param(fit->stack_list[sample], fp, val);
    }

    return res;

}

int
multi_fit_engine_commit_parameters(struct multi_fit_engine *fit,
                                   const gsl_vector *x)
{
    struct fit_parameters const * common = fit->common_parameters;
    struct fit_parameters const * priv   = fit->private_parameters;
    size_t nb_priv_params = priv->number;
    size_t j, joffs = 0;
    int sample;

    for(j = 0; j < common->number; j++) {
        const fit_param_t *fp = common->values + j;
        double param_value = gsl_vector_get(x, joffs + j);
        int status;

        status = mengine_apply_param_common(fit, fp, param_value);

        /* No error should never occurs here because the fit parameters
        are checked in advance. */
        assert(status == 0);
    }

    joffs += common->number;

    for(j = 0; j < nb_priv_params; j++) {
        const fit_param_t *fp = priv->values + j;

        for(sample = 0; sample < fit->samples_number; sample++) {
            double param_value =
                gsl_vector_get(x, joffs + nb_priv_params * sample + j);
            int status;

            status = mengine_apply_param_priv(fit, fp, param_value, sample);

            assert(status == 0);
        }
    }

    return 0;
}

struct multi_fit_engine *
multi_fit_engine_new(struct fit_config const *cfg, int samples_number) {
    struct multi_fit_engine *f;

    f = emalloc(sizeof(struct multi_fit_engine));

    f->system_kind = SYSTEM_UNDEFINED;

    f->samples_number = samples_number;
    f->stack_list = emalloc(samples_number * sizeof(void *));
    f->spectra_list = emalloc(samples_number * sizeof(void *));

    set_default_extra_param(& f->extra);

    memcpy(&f->config, cfg, sizeof(struct fit_config));

    /* fit parameters are provided later */
    f->common_parameters = NULL;
    f->private_parameters = NULL;

    f->results = NULL;

    f->initialized = 0;

    return f;
}

void
multi_fit_engine_free(struct multi_fit_engine *f)
{
    int k;

    if(f->stack_list) {
        for(k = 0; k < f->samples_number; k++) {
            if(f->stack_list[k] != NULL) {
                stack_free(f->stack_list[k]);
            }
        }
        free(f->stack_list);
    }


    /* we don't free each spectrum because we are not the owner */
    free(f->spectra_list);

    /* we don't free f->common_parameters and f->private_parameters
       because multi_fit_engine does not own these data */

    assert(f->initialized == 0);

    free(f);
}

void
multi_fit_engine_bind(struct multi_fit_engine *fit, const stack_t *stack, struct fit_parameters *cparameters, struct fit_parameters *pparameters)
{
    int k;
    /* fit is not the owner of the "parameters", we just keep a reference */
    fit->common_parameters = cparameters;

    for(k = 0; k < fit->samples_number; k++) {
        fit->stack_list[k] = stack_copy(stack);
    }

    /* fit is not the owner of the "parameters", we just keep a reference */
    fit->private_parameters = pparameters;
}

void
multi_fit_engine_apply_parameters(struct multi_fit_engine *fit, const struct fit_parameters *fps, const double value[])
{
    int k;
    for(k = 0; k < fps->number; k++) {
        stack_apply_param(fit->stack_list[k], fps->values + k, value[k]);
    }
}

extern struct multi_fit_engine *
build_multi_fit_engine(struct symtab *symtab, struct seeds **comm,
                       struct seeds **indiv) {
    stack_t *stack;
    struct strategy *strategy;
    struct multi_fit_info *inf;
    struct multi_fit_engine *fit;
    int fit_parameters_error;
    double *seed_array;
    int j, k, index;

    stack    = retrieve_parsed_object(symtab, TL_TYPE_STACK,
                                      symtab->directives->stack);
    strategy = retrieve_parsed_object(symtab, TL_TYPE_STRATEGY,
                                      symtab->directives->strategy);
    inf      = retrieve_parsed_object(symtab, TL_TYPE_MULTI_FIT,
                                      symtab->directives->multi_fit);

    if(stack == NULL || strategy == NULL || inf == NULL) {
        return NULL;
    }

    fit_parameters_error =					       \
            check_fit_parameters(stack, strategy->parameters) ||	       \
            check_fit_parameters(stack, inf->constraints.parameters) ||       \
            check_fit_parameters(stack, inf->individual.parameters);

    if(fit_parameters_error) {
        notify_error_msg(SCRIPT_ERROR,
                         "some of the fit parameters are not valid");
        return NULL;
    }

    *comm  = strategy->seeds;
    *indiv = inf->individual.seeds;

    fit = multi_fit_engine_new(symtab->config_table, inf->samples_number);
    multi_fit_engine_bind(fit, stack, strategy->parameters, inf->individual.parameters);

    struct fit_parameters *constr = inf->constraints.parameters;
    seed_array = emalloc(constr->number * sizeof(double));

    for(index = 0, k = 0; k < fit->samples_number; k++) {
        for(j = 0; j < constr->number; j++, index++) {
            seed_array[j] = inf->constraints.seeds->values[index].seed;
        }
        multi_fit_engine_apply_parameters(fit, constr, seed_array);
    }
    free(seed_array);

    for(k = 0; k < fit->samples_number; k++) {
        /* we just keep a reference, we don't own the data */
        fit->spectra_list[k] = inf->spectra_list[k];
    }

    if(multi_fit_engine_prepare(fit) != 0) {
        multi_fit_engine_free(fit);
        return NULL;
    }

    return fit;
}

void
multi_fit_engine_print_fit_results(struct multi_fit_engine *fit,
                                   str_t text)
{
    str_t pname;
    size_t k, j, kp;

    str_set_null(text);

    str_init(pname, 16);

    for(j = 0, kp = 0; j < fit->common_parameters->number; j++, kp++) {
        fit_param_t *fp = fit->common_parameters->values + j;
        get_param_name(fp, pname);
        str_printf_add(text, "COMMON    / %9s : %.6g\n", CSTR(pname),
                       gsl_vector_get(fit->results, j));
    }


    for(k = 0; k < fit->samples_number; k++) {
        for(j = 0; j < fit->private_parameters->number; j++, kp++) {
            fit_param_t *fp = fit->private_parameters->values + j;
            get_param_name(fp, pname);
            str_printf_add(text, "SAMPLE(%02i)/ %9s : %.6g\n", k, CSTR(pname),
                           gsl_vector_get(fit->results, kp));
        }
    }

    str_append_c(text, "Residual Chi Square by sample:\n", '\n');
    for(k = 0; k < fit->samples_number; k++) {
        double chisq = gsl_vector_get(fit->chisq, k);
        str_printf_add(text, "ChiSq(%02i): %g\n", k, chisq);
    }

    str_free(pname);
}
