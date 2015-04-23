#include <assert.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_blas.h>

#include "lmfit.h"
#include "lmfit-simple.h"
#include "stack.h"
#include "vector_print.h"

int
lmfit_simple(struct fit_engine *fit, gsl_vector *x,
             double * chisq, str_ptr analysis, str_ptr error_msg,
             gui_hook_func_t hfun, void *hdata)
{
    const gsl_multifit_fdfsolver_type *T;
    gsl_multifit_fdfsolver *s;
    gsl_multifit_function_fdf *f;
    struct fit_config *cfg = fit->config;
    int iter;
    double chi;
    int status;
    int stop_request;

    assert(fit->run);

    f = &fit->run->mffun;

    /* We choose Levenberg-Marquardt algorithm, scaled version*/
    T = gsl_multifit_fdfsolver_lmsder;
    s = gsl_multifit_fdfsolver_alloc(T, f->n, f->p);

    if(analysis) {
        str_copy_c(analysis, "Seed used: ");
        print_vector(analysis, "%.5g", x);
    }

    status = lmfit_iter(x, f, s, cfg->nb_max_iters, cfg->epsabs, cfg->epsrel,
                        & iter, hfun, hdata, & stop_request);

    chi = gsl_blas_dnrm2(s->f);
    *chisq = 1.0E6 * pow(chi, 2.0) / f->n;

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
        print_analysis(analysis, f, s);
    }

    if(stop_request) {
        status = 1;
        if(error_msg) {
            str_copy_c(error_msg, "Fit interrupted by user request.");
        }
        if(analysis) {
            str_copy_c(analysis, "** Fit interrupted by the user.");
        }
    }

    /* we take care to commit the results obtained from the fit */
    fit_engine_commit_parameters(fit, x);

    gsl_vector_memcpy(fit->run->results, x);

    gsl_multifit_fdfsolver_free(s);

    return status;
}
