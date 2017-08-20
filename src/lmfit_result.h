#ifndef LMFIT_RESULT_H
#define LMFIT_RESULT_H

#include "defs.h"

/* The GSL error codes above 1024 can be used by applications. So
   we define some error codes compatible with GSL status codes. */
enum {
    LMFIT_DATA_NOT_LOADED = 1024,
    LMFIT_USER_INTERRUPTED,
};

struct lmfit_result {
    float chisq;
    int nb_points;
    int nb_iterations;
    int gsl_status;
};

const char *lmfit_result_error_string(const struct lmfit_result *r);

#endif
