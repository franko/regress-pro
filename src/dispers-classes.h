
#ifndef DISPERS_CLASSES_H
#define DISPERS_CLASSES_H

#include "defs.h"

__BEGIN_DECLS

enum disp_type {
    DISP_UNSET = 0,
    DISP_TABLE,
    DISP_SAMPLE_TABLE,
    DISP_CAUCHY,
    DISP_HO,
    DISP_LOOKUP,
    DISP_BRUGGEMAN,
    DISP_FB,
    DISP_TAUC_LORENTZ,
    DISP_SELLMEIER,
    DISP_LORENTZ,
    DISP_END_OF_TABLE, /* Not a dispersion type */
};

struct disp_class;

/* used to keep a linked list of dispersion classes */
struct disp_class_node {
    struct disp_class *value;
    struct disp_class_node *next;
};

extern struct disp_class_node *disp_class_list;

extern int                 init_class_list(void);
extern struct disp_class * disp_class_lookup(int tp);

/* Enumerate all the possible dispersion classes using the iterator.
   When iter is NULL the iteration starts. When the iteration is over
   return NULL. */
extern void * disp_class_next(void *iter);
extern struct disp_class * disp_class_from_iter(void *iter);

__END_DECLS

#endif
