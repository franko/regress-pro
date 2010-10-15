#ifndef DISP_CAUCHY_H
#define DISP_CAUCHY_H

#include "dispers-classes.h"

__BEGIN_DECLS

struct disp_cauchy {
  double n[3];
  double k[3];
};

struct disp_struct;

/* Cauchy dispersion class */
extern struct disp_class cauchy_disp_class;

extern struct disp_struct * disp_new_cauchy (const char *name,
					     const double n[],
					     const double k[]);

__END_DECLS

#endif
