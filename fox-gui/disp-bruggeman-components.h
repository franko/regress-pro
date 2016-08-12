#ifndef DISP_BRUGGEMAN_COMPONENTS_H
#define DISP_BRUGGEMAN_COMPONENTS_H

#include "disp-components.h"

class disp_bruggeman_components : public disp_components {
public:
    disp_bruggeman_components(disp_t *d): m_disp(d) { }

    const disp_bruggeman& bema() const { return m_disp->disp.bruggeman; }
          disp_bruggeman& bema()       { return m_disp->disp.bruggeman; }

    const bema_component& component(int i) const { return bema().components[i]; }
          bema_component& component(int i)       { return bema().components[i]; }

    int length() const override { return bema().components_number + 1; }

    // Not used.
    double *map_component_value(int i) override { return nullptr; }

    disp_t *disp(int i) override { return (i == 0 ? bema().disp_base : component(i-1).disp); }

    void assign_disp(int i, disp_t *new_disp) override {
        if (i == 0) {
            bema().disp_base = new_disp;
        } else {
            component(i-1).disp = new_disp;
        }
    }

    void insert(int i, disp_t *new_disp) override {
        if (i == 0) {
            bruggeman_add_comp(m_disp, 0, new_disp);
            std::swap(bema().disp_base, component(0).disp);
        } else {
            bruggeman_add_comp(m_disp, i-1, new_disp);
        }
    }

    void remove(int i) override {
        if (i == 0) {
            std::swap(bema().disp_base, component(0).disp);
            bruggeman_delete_comp(m_disp, 0);
        } else {
            bruggeman_delete_comp(m_disp, i-1);
        }
    }

private:
    disp_t *m_disp;
};

#endif
