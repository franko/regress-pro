#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "common.h"
#include "lmfit.h"
#include "str.h"

#include <gsl/gsl_blas.h>
#include <gsl/gsl_eigen.h>

static void covar_analysis(str_t msg, gsl_matrix *covar);

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

void
print_vector(str_t s, const char *fmt, const gsl_vector *v)
{
    size_t j;
    for(j = 0; j < v->size; j++) {
        if(j > 0) {
            str_append_c(s, " ", 0);
        }
        str_printf_add(s, fmt, gsl_vector_get(v, j));
    }
    str_append_c(s, "\n", 0);
}

void
print_matrix(str_t s, const char *fmt, const gsl_matrix *m)
{
    int field_size = 0;
    size_t i, j;
    size_t n1 = m->size1, n2 = m->size2;
    str_t number;

    str_init(number, 64);

    for(i = 0; i < n1; i++) {
        for(j = 0; j < n2; j++) {
            str_printf(number, fmt, gsl_matrix_get(m, i, j));
            if(STR_LENGTH(number) > field_size) {
                field_size = STR_LENGTH(number);
            }
        }
    }

    for(i = 0; i < n1; i++) {
        str_append_c(s, "|", 0);
        for(j = 0; j < n2; j++) {
            if(j > 0) {
                str_append_c(s, " ", 0);
            }

            str_printf(number, fmt, gsl_matrix_get(m, i, j));
            str_pad(number, field_size, ' ');

            str_append(s, number, 0);
        }
        str_append_c(s, "|\n", 0);
    }

    str_free(number);
}

void
covar_analysis(str_t msg, gsl_matrix *covar)
{
    size_t n = covar->size1;
    gsl_vector *eval;
    gsl_matrix *evec, *corr;
    gsl_eigen_symmv_workspace *w;
    size_t i, j;

    eval = gsl_vector_alloc(n);
    evec = gsl_matrix_alloc(n, n);
    corr = gsl_matrix_alloc(n, n);

    for(i = 0; i < n; i++)
        for(j = 0; j < n; j++) {
            double v  = gsl_matrix_get(covar, i, j);
            double si = gsl_matrix_get(covar, i, i);
            double sj = gsl_matrix_get(covar, j, j);

            gsl_matrix_set(corr, i, j, v/sqrt(si*sj));
        }

    str_append_c(msg, "Covar. matrix:\n", '\n');
    print_matrix(msg, "%.5g", covar);

    str_append_c(msg, "Correlation matrix:\n", 0);
    print_matrix(msg, "%.5g", corr);

    w = gsl_eigen_symmv_alloc(n);
    gsl_eigen_symmv(covar, eval, evec, w);

    str_append_c(msg, "\nEigenvalues:\n", 0);
    print_vector(msg, "%.5g", eval);

    str_append_c(msg, "Eigenvectors (columns):\n", 0);
    print_matrix(msg, "%.4f", evec);

    gsl_matrix_free(evec);
    gsl_matrix_free(corr);
    gsl_vector_free(eval);
    gsl_eigen_symmv_free(w);
}

void
print_analysis(str_t msg, gsl_multifit_function_fdf *f,
               gsl_multifit_fdfsolver *s)
{
    gsl_matrix *covar;
#if 0
    size_t j;

    for(j = 0; j < f->p; j++) {
        str_append_c(msg, "\n", 0);
        str_printf_add(msg, "PARAM(%i) = %.5f", j, gsl_vector_get(s->x, j));
    }
#endif
    covar = gsl_matrix_alloc(f->p, f->p);
    gsl_multifit_covar(s->J, 1e-6, covar);
    covar_analysis(msg, covar);
    gsl_matrix_free(covar);
}

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
