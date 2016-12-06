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
            stop_request = (*hfun)(hdata, iter / (float)max_iter, NULL);
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
