#ifndef DISP_LOOKUP_H
#define DISP_LOOKUP_H

#include "defs.h"
#include "dispers-classes.h"

__BEGIN_DECLS

struct disp_struct;

struct lookup_comp {
    double p;
    struct disp_struct * disp;
};

struct disp_lookup {
    int nb_comps;
    struct lookup_comp * component;
    double p;
};

extern struct disp_class disp_lookup_class;

extern struct disp_struct *
disp_new_lookup(const char *name, int nb_comps, struct lookup_comp *comp,
                double p0);

extern void disp_lookup_add_comp(struct disp_struct *d, struct disp_struct *comp, double p);
extern void disp_lookup_delete_comp(struct disp_struct *d, int index);

__END_DECLS

#endif
