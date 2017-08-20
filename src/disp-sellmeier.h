#ifndef DISP_SELLMEIER_H
#define DISP_SELLMEIER_H

#include "dispers-classes.h"

struct disp_sellmeier {
    double a[3];
    double b[3];
};

struct disp_struct;

extern struct disp_class sellmeier_disp_class;

extern struct disp_struct * disp_new_sellmeier(const char *name,
        const double a[],
        const double b[]);

#endif
