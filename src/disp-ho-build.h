#ifndef DISP_HO_BUILD_H
#define DISP_HO_BUILD_H

#include <stdlib.h>

#include "dispers.h"

template <typename OscillatorParameters>
disp_t * new_ho(const char *name, const int osc_num, OscillatorParameters osc) {
    ho_params *params = (ho_params *) malloc(osc_num * sizeof(ho_params));
    if (!params) return nullptr;
    disp_t *d = disp_new_with_name(DISP_HO, name);
    d->disp.ho.nb_hos = osc_num;
    d->disp.ho.params = params;
    for (int i = 0, base = 0; i < osc_num; i++, base += 5) {
        params[i].nosc = osc(base + 0);
        params[i].en   = osc(base + 1);
        params[i].eg   = osc(base + 2);
        params[i].nu   = osc(base + 3);
        params[i].phi  = osc(base + 4);
    }
    return d;
}

#endif
