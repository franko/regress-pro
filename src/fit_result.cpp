#include "fit_result.h"
#include "str.h"
#include "vector_print.h"

void
fit_result_init(struct fit_result *r, struct fit_engine *fit)
{
    size_t p = fit->parameters->number;
    r->gsearch_x = gsl_vector_alloc(p);
}

void
fit_result_free(struct fit_result *r)
{
    gsl_vector_free(r->gsearch_x);
}

void
fit_result_report(struct fit_result *r, str_ptr analysis)
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
}
