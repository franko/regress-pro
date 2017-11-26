#ifndef REGPRO_NLOPT_FIT_H
#define REGPRO_NLOPT_FIT_H

#include "fit-engine.h"
#include "fit-params.h"
#include "lmfit.h"
#include "lmfit_result.h"
#include "str.h"

extern void nlopt_fit(fit_engine *fit, spectrum *spectrum, gsl::vector& x, seeds *seeds, lmfit_result *result, str_ptr analysis, int preserve_init_stack, gui_hook_func_t hfun, void *hdata);

#endif
