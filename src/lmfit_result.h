#ifndef LMFIT_RESULT_H
#define LMFIT_RESULT_H

#include "defs.h"

__BEGIN_DECLS

/* The GSL error codes above 1024 can be used by applications. So
   we define some error codes compatible with GSL status codes. */
enum {
    LMFIT_DATA_NOT_LOADED = 1024,
};

struct lmfit_result {
    float chisq;
    int nb_iterations;
    int gsl_status;
};

__END_DECLS

#endif
