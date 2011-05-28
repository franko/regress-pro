#ifndef DISP_HO_H
#define DISP_HO_H

#include "defs.h"
#include "dispers-classes.h"

__BEGIN_DECLS

struct ho_params {
  double nosc;
  double en;
  double eg;
  double nu;
  double phi;
};

struct disp_ho {
  int nb_hos;
  struct ho_params *params;
};

struct disp_struct;

/* HO dispersion class */
extern struct disp_class ho_disp_class;

extern struct disp_struct * disp_new_ho (const char *name, int nb_hos, 
					 struct ho_params *params);

__END_DECLS

#endif
