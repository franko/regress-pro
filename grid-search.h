
/*
  $Id$
 */

#ifndef GRID_SEARCH_H
#define GRID_SEARCH_H

#include <gsl/gsl_vector.h>

#include "lmfit.h"
#include "fit-engine.h"
#include "str.h"

__BEGIN_DECLS

/* returns NULL if not successfull */
int lmfit_grid (struct fit_engine *fit, struct seeds *seeds,
		double * chisq, str_ptr analysis,
		str_ptr error_msg, int preserve_init_stack,
		gui_hook_func_t hfun, void *hdata);

__END_DECLS

#endif
