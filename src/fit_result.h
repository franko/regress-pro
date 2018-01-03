#ifndef FIT_RESULT_H
#define FIT_RESULT_H

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

#include "fit-engine.h"

struct fit_result {
    double gsearch_chisq;
    double chisq_threshold;
    gsl_vector *gsearch_x;
    int status;
    int interrupted;
    double chisq;
};

extern void fit_result_init(struct fit_result *r, struct fit_engine *fit);
extern void fit_result_free(struct fit_result *r);
extern void fit_result_report(struct fit_result *r, str_ptr analysis);

#endif
