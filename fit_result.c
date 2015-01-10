#include "fit_result.h"
#include "str.h"
#include "lmfit.h"

void
fit_result_init(struct fit_result *r, struct fit_engine *fit, int want_cover_mat)
{
    size_t p = fit->parameters->number;
    r->gsearch_x = gsl_vector_alloc(p);
    if (want_cover_mat) {
        r->covar = gsl_matrix_alloc(p, p);
    } else {
        r->covar = NULL;
    }
}

void
fit_result_free(struct fit_result *r)
{
    gsl_vector_free(r->gsearch_x);
    if (r->covar) {
        gsl_matrix_free(r->covar);
    }
}

void
fit_result_report(struct fit_result *r, str_ptr analysis, str_ptr error)
{
    if (r->gsearch_chisq >= 0.0) {
        str_copy_c(analysis, "Grid search best point: ");
        print_vector(analysis, "%.5g", r->gsearch_x);
        str_printf_add(analysis, "With Chi Square: %g. Required threshold was: %g.\n\n", r->gsearch_chisq, r->chisq_threshold);
    }

    if (r->interrupted) {
        str_copy_c(analysis, "** Fit interrupted by the user.\n");
        return;
    }

    switch(r->status) {
    case GSL_SUCCESS:
        str_trunc(error, 0);
        break;
    case GSL_CONTINUE:
        str_copy_c(error, "Error: more iterations needed.");
        break;
    default:
        str_printf(error, "Error: %s", gsl_strerror(r->status));
    }

    str_printf_add(analysis, "Nb of iterations to converge: %i\n", r->iter);
    if (r->covar) {
        str_append_c(analysis, "\nCovariance matrix:\n", 0);
        print_matrix(analysis, "%g", r->covar);
    }
}
