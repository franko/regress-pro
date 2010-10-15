
/*
  $Id: elliss-util.h,v 1.3 2006/07/12 22:48:54 francesco Exp $
 */

#ifndef ELLISS_UTIL_H
#define ELLISS_UTIL_H

#include "elliss.h"
#include "spectra.h"

extern struct spectrum * load_ellips_spectrum (const char *filename,
					       struct extra_param *einf);

#endif
