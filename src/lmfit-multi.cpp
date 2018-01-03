#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_vector.h>
#include <assert.h>

#include "str.h"
#include "lmfit.h"
#include "lmfit-multi.h"
#include "fit-params.h"
#include "multi-fit-engine.h"
#include "vector_print.h"


int
lmfit_multi(struct multi_fit_engine *fit,
            seeds_list *seeds_common, seeds_list *seeds_priv,
            str_ptr analysis, str_ptr error_msg,
            gui_hook_func_t hfun, void *hdata)
{
    const gsl_multifit_fdfsolver_type *T;
    gsl_multifit_fdfsolver *s;
    gsl_multifit_function_fdf *f = & fit->mffun;
    struct fit_config *cfg = &fit->config;
    int status, stop_request = 0;
    gsl_vector *x;
    int iter, nb_common, nb_priv, nb_samples, k, ks;

    nb_samples = fit->samples_number;
    nb_common  = fit->common_parameters->number;
    nb_priv    = fit->private_parameters->number;

    x = gsl_vector_alloc(nb_common + nb_priv * nb_samples);

    T = gsl_multifit_fdfsolver_lmsder;
    s = gsl_multifit_fdfsolver_alloc(T, f->n, f->p);

    for(k = 0; k < seeds_common->number; k++) {
        gsl_vector_set(x, k, multi_fit_engine_get_seed_value(fit, &fit->common_parameters->at(k), &seeds_common->at(k)));
    }

    for(ks = 0; ks < seeds_priv->number; ks++, k++) {
        gsl_vector_set(x, k, seeds_priv->at(ks).seed);
    }

    if(analysis) {
        str_copy_c(analysis, "Seed used: ");
        print_vector(analysis, "%.5f", x);
    }

    status = lmfit_iter(x, f, s, cfg->nb_max_iters,
                        cfg->epsabs, cfg->epsrel,
                        & iter, hfun, hdata, & stop_request);

    multi_fit_engine_compute_chisq(fit, s->f);

    if(error_msg) {
        switch(status) {
        case GSL_SUCCESS:
            str_trunc(error_msg, 0);
            break;
        case GSL_CONTINUE:
            str_copy_c(error_msg, "Error: more iterations needed.");
            break;
        default:
            str_printf(error_msg, "Error: %s", gsl_strerror(status));
        }
    }

    if(analysis && !stop_request) {
        str_printf_add(analysis, "Nb of iterations to converge: %i\n", iter);
    }

    gsl_multifit_fdfsolver_free(s);

    if(stop_request) {
        status = 1;
        if(error_msg) {
            str_copy_c(error_msg, "Fit interrupted by user request.");
        }
        if(analysis) {
            str_copy_c(analysis, "** Fit interrupted by the user.");
        }
    }

    /* we take care to commit the last results obtained from the fit */
    multi_fit_engine_commit_parameters(fit, x);

    assert(fit->results != nullptr);
    gsl_vector_memcpy(fit->results, x);

    gsl_vector_free(x);

    return status;
}
