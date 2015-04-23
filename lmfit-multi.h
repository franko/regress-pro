
/*
  $Id$
 */

#ifndef LMFIT_MULTI_H
#define LMFIT_MULTI_H

#include "lmfit.h"
#include "multi-fit-engine.h"
#include "str.h"

__BEGIN_DECLS

/* returns 0 if successfull */
int
lmfit_multi(struct multi_fit_engine *fit,
            struct seeds *seeds_common, struct seeds *seeds_priv,
            str_ptr analysis, str_ptr error_msg,
            gui_hook_func_t hfun, void *hdata);

__END_DECLS

#endif
