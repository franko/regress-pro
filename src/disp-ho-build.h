#ifndef DISP_HO_BUILD_H
#define DISP_HO_BUILD_H

#include <stdlib.h>

#include "dispers.h"

template <typename OscillatorParameters>
disp_t * new_ho(const char *name, OscillatorParameters osc) {
    const int osc_num = osc.number();
    ho_params *params = (ho_params *) malloc(osc_num * sizeof(ho_params));
    if (!params) return nullptr;
    disp_t *d = disp_new_with_name(DISP_HO, name);
    d->disp.ho.nb_hos = osc_num;
    d->disp.ho.params = params;
    for (int i = 0; i < osc_num; i++) {
        params[i].nosc = osc.parameter(i, 0);
        params[i].en   = osc.parameter(i, 1);
        params[i].eg   = osc.parameter(i, 2);
        params[i].nu   = osc.parameter(i, 3);
        params[i].phi  = osc.parameter(i, 4);
    }
    return d;
}

#endif
