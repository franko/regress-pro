#ifndef DISP_BRUGGEMAN_H
#define DISP_BRUGGEMAN_H

struct disp_struct;

struct disp_bruggeman {
  /* two components Bruggeman EMA model */
  double frac[2];
  struct disp_struct *comp[2];
};

extern struct disp_class bruggeman_disp_class;

#endif
