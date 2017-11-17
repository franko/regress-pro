#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "common.h"
#include "lmfit.h"

#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlin.h>

int
lmfit_iter(gsl_vector *x, gsl_multifit_function_fdf *f,
           gsl_multifit_fdfsolver *s, const int max_iter,
           double epsabs, double epsrel, int *nb_iter,
           gui_hook_func_t hfun, void *hdata, int *user_stop)
{
    int iter = 0, status;
    int stop_request = 0;

    gsl_multifit_fdfsolver_set(s, f, x);

    if(hfun) {
        stop_request = (*hfun)(hdata, 0.0, "Running Levenberg-Marquardt search...");
    }

    do {
        if(hfun) {
            stop_request = (*hfun)(hdata, iter / (float)max_iter, nullptr);
        }

        iter++;
        status = gsl_multifit_fdfsolver_iterate(s);

        if(status) {
            break;
        }

        status = gsl_multifit_test_delta(s->dx, s->x, epsabs, epsrel);
    } while(status == GSL_CONTINUE && iter < max_iter && !stop_request);

    memcpy(x->data, s->x->data, f->p * sizeof(double));

    *nb_iter = iter;
    if(user_stop) {
        *user_stop = stop_request;
    }

    return status;
}

// Perform Levenberg-Marquart final step. Store the new results in gsl_vector x and update
// result struct.
void fit_engine_lmfit(fit_engine *fit, gsl_vector *x, lmfit_result *result, fit_config *cfg, gui_hook_func_t hfun, void *hdata, int& stop_request) {
    gsl_multifit_function_fdf *f = &fit->run->mffun;
    const gsl_multifit_fdfsolver_type *T = gsl_multifit_fdfsolver_lmsder;
    gsl_multifit_fdfsolver *s = gsl_multifit_fdfsolver_alloc(T, f->n, f->p);

    int iter;
    int status = lmfit_iter(x, f, s, cfg->nb_max_iters, cfg->epsabs, cfg->epsrel, &iter, hfun, hdata, &stop_request);

    const double chi = gsl_blas_dnrm2(s->f);
    result->chisq = 1.0E6 * pow(chi, 2.0) / f->n;
    result->gsl_status = status;
    result->nb_iterations = iter;

    gsl_multifit_fdfsolver_free(s);
}
