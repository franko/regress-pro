#include "disp-lookup-components.h"

void disp_lookup_components::insert(int i, disp_t *new_disp) {
    double new_p;
    const int n = length();
    if (i > 0) {
        double p1 = component(i-1).p, p2 = component(i).p;
        new_p = (p1 + p2) / 2.0;
    } else {
        double p1 = component(0).p, p2 = component(n-1).p;
        new_p = p1 - (n > 1 ? (p2 - p1) / (n - 1) : 1.0);
    }
    disp_lookup_add_comp(m_disp, i, new_disp, new_p);
}

void disp_lookup_components::remove(int i) {
    disp_lookup_delete_comp(m_disp, i);
}
