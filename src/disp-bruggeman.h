#ifndef DISP_BRUGGEMAN_H
#define DISP_BRUGGEMAN_H

#include "defs.h"

__BEGIN_DECLS

struct disp_struct;

struct bema_component {
    double fraction;
    struct disp_struct *disp;
};

struct disp_bruggeman {
    struct disp_struct *disp_base;
    int components_number;
    struct bema_component *components;
};

extern struct disp_class bruggeman_disp_class;

extern struct disp_struct *bruggeman_new(const char *name, struct disp_struct *disp_base, int components_number, struct bema_component *components);

__END_DECLS

#endif
