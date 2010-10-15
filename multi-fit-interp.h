
/*
  $Id$
 */

#ifndef MULTI_FIT_INTERP_H
#define MULTI_FIT_INTERP_H

#include "interp.h"

__BEGIN_DECLS

extern obj_t * sample_interp    (struct symtab *symtab, struct gen_record *r);
extern obj_t * multi_fit_interp (struct symtab *symtab, struct gen_record *r);

__END_DECLS

#endif
