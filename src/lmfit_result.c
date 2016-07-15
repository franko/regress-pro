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
