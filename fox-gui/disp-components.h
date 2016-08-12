#ifndef DISP_COMPONENTS_H
#define DISP_COMPONENTS_H

#include "dispers.h"

struct disp_components {
    disp_components() { }
    virtual ~disp_components() { }
    virtual int length() const = 0;
    virtual double *map_component_value(int i) = 0;
    virtual disp_t *disp(int i) = 0;
    virtual void assign_disp(int i, disp_t *new_disp) = 0;
    virtual void insert(int i, disp_t *new_disp) = 0;
    virtual void remove(int i) = 0;
};

#endif
