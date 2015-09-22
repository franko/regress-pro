#ifndef LMFIT_SIMPLE_H
#define LMFIT_SIMPLE_H

#include <gsl/gsl_vector.h>

#include "lmfit.h"
#include "fit-engine.h"
#include "lmfit_result.h"
#include "str.h"

__BEGIN_DECLS

/* returns NULL if not successfull */
int
lmfit_simple(struct fit_engine *fit, gsl_vector *x,
             struct lmfit_result *result,
             str_ptr analysis, str_ptr error_msg,
             gui_hook_func_t hfun, void *hdata);

__END_DECLS

#endif
