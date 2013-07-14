#ifndef DISP_UTIL_H
#define DISP_UTIL_H

#include "dispers.h"

__BEGIN_DECLS

extern int write_mat_file(const char *filename, const disp_t *d,
                          double lmin, double lmax, double step);

__END_DECLS

#endif
