#include <math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_eigen.h>

#include "covariance_analysis.h"
#include "vector_print.h"

void
print_covar_analysis(str_t msg, gsl_matrix *covar)
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
