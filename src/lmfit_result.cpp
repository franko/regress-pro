#include <gsl/gsl_errno.h>

#include "lmfit_result.h"

const char *
lmfit_result_error_string(const struct lmfit_result *r)
{
    if (r->gsl_status == LMFIT_DATA_NOT_LOADED) {
        return "no data loaded";
    } else if (r->gsl_status == LMFIT_USER_INTERRUPTED) {
        return "fit interrupted by user";
    }
    return gsl_strerror(r->gsl_status);
}

void lmfit_result_error_init(lmfit_result *result, int gsl_status) {
    result->chisq = -1.0;
    result->nb_points = -1;
    result->nb_iterations = -1;
    result->gsl_status = gsl_status;
}
