#ifndef ELLISS_UTIL_H
#define ELLISS_UTIL_H

#include "elliss.h"
#include "spectra.h"

extern struct spectrum * load_ellips_spectrum(const char *filename,
        struct extra_param *einf);

#endif
