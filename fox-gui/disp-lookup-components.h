#ifndef DISP_LOOKUP_COMPONENTS_H
#define DISP_LOOKUP_COMPONENTS_H

#include "disp-components.h"

class disp_lookup_components : public disp_components {
public:
    disp_lookup_components(disp_t *d): m_disp(d) { }

    const disp_lookup& lookup() const { return m_disp->disp.lookup; }
          disp_lookup& lookup()       { return m_disp->disp.lookup; }

    const lookup_comp& component(int i) const { return lookup().component[i]; }
          lookup_comp& component(int i)       { return lookup().component[i]; }

    int length() const override { return lookup().nb_comps; }

    double *map_component_value(int i) override { return &component(i).p; }

    disp_t *disp(int i) override { return component(i).disp; }

    void assign_disp(int i, disp_t *new_disp) override { component(i).disp = new_disp; }

    void insert(int i, disp_t *new_disp) override;
    void remove(int i) override;

private:
    disp_t *m_disp;
};

#endif
