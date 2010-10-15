
/*
  $Id: minsampling.h,v 1.1 2006/07/12 22:57:45 francesco Exp $
 */

#ifndef MINSAMPLING_H
#define MINSAMPLING_H

#include "spectra.h"

__BEGIN_DECLS

extern void elliss_sample_minimize (struct spectrum *s, float dlmt);

// extern struct spectrum *minsampling_ell (struct spectrum *src, double dlmt);

__END_DECLS

#endif
