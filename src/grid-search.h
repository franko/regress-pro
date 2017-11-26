#ifndef GRID_SEARCH_H
#define GRID_SEARCH_H

#include <gsl/gsl_vector.h>

#include "lmfit.h"
#include "fit-engine.h"
#include "lmfit_result.h"
#include "str.h"

/* returns NULL if not successfull */
int lmfit_grid(fit_engine *fit, spectrum *spectrum, gsl::vector& x, seeds *seeds,
               lmfit_result *result, str_ptr analysis, int preserve_init_stack,
               gui_hook_func_t hfun, void *hdata);

#endif
