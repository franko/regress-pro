#ifndef REFL_UTILS_H
#define REFL_UTILS_H

#include "spectra.h"
#include "refl-kernel.h"

extern struct spectrum * load_refl_data(const char *filename, str_ptr *error_msg);
extern struct spectrum * load_filmetrics_spectrum(const char *filename, str_ptr *error_msg);

#endif
