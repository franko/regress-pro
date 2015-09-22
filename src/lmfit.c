#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "common.h"
#include "lmfit.h"

#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlin.h>

#ifdef DEBUG_REGRESS
#define FIT_DEBUG
#endif

#ifdef FIT_DEBUG
static void
print_state(int iter, gsl_multifit_fdfsolver * s, unsigned int dof)
{
    unsigned int j, nb = s->fdf->p;

    printf("iter: %3u x = ", iter);
    for(j = 0; j < nb; j++) {
        printf("%15.8f ", gsl_vector_get(s->x, j));
    }
    printf("Chi^2 = %g, DoF= %i\n", 1.0E6 * pow(gsl_blas_dnrm2(s->f), 2.0) / dof, dof);
}

static void
print_vector_stdout(const char *fmt, const gsl_vector *v)
{
    size_t j;
    for(j = 0; j < v->size; j++) {
        if(j > 0) {
            fputc(' ', stdout);
        }
        printf(fmt, gsl_vector_get(v, j));
    }
}
#endif

int
lmfit_iter(gsl_vector *x, gsl_multifit_function_fdf *f,
           gsl_multifit_fdfsolver *s, const int max_iter,
           double epsabs, double epsrel, int *nb_iter,
           gui_hook_func_t hfun, void *hdata, int *user_stop)
{
    int iter = 0, status;
    int stop_request = 0;
#ifdef FIT_DEBUG
    gsl_vector *grad;
#endif

    gsl_multifit_fdfsolver_set(s, f, x);

#ifdef FIT_DEBUG
    print_state(iter, s, f->n);
    grad = gsl_vector_alloc(f->p);
#endif

    if(hfun) {
        stop_request = (*hfun)(hdata, 0.0, "Running Levenberg-Marquardt search...");
    }

    do {
        if(hfun) {
            stop_request = (*hfun)(hdata, iter / (float)max_iter, NULL);
        }

        iter++;
        status = gsl_multifit_fdfsolver_iterate(s);

#ifdef FIT_DEBUG
        printf("status = %s\n", gsl_strerror(status));
        print_state(iter, s, f->n);
#endif

        if(status) {
            break;
        }

        status = gsl_multifit_test_delta(s->dx, s->x, epsabs, epsrel);

#ifdef FIT_DEBUG
        gsl_multifit_gradient(s->J, s->f, grad);
        printf("gradient: ");
        print_vector_stdout(" %f", grad);
        printf("\n");
        fflush(stdout);
#endif
    } while(status == GSL_CONTINUE && iter < max_iter && !stop_request);

#ifdef FIT_DEBUG
    printf("final status = %s\n", gsl_strerror(status));
    gsl_vector_free(grad);
#endif

    memcpy(x->data, s->x->data, f->p * sizeof(double));

    *nb_iter = iter;
    if(user_stop) {
        *user_stop = stop_request;
    }

    return status;
}
