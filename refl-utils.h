#ifndef REFL_UTILS_H
#define REFL_UTILS_H

#include "spectra.h"
#include "refl-kernel.h"

__BEGIN_DECLS

extern struct spectrum * load_refl_data(const char *filename, str_ptr *error_msg);

__END_DECLS

#endif
