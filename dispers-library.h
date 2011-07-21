/*
  $Id$
 */

#ifndef DISPERS_LIBRARY_H
#define DISPERS_LIBRARY_H

#include "defs.h"
#include "dispers.h"

__BEGIN_DECLS

extern void     dispers_library_init      (void);
extern disp_t * dispers_library_search    (const char *id);
extern disp_t * dispers_library_get       (int index, char const ** lname);

__END_DECLS

#endif
