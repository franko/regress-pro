#ifndef SPECTRUM_VS_H
#define SPECTRUM_VS_H

#include "spectra.h"
#include "vs_object.h"

class spectrum_base_vs {
public:
    spectrum_base_vs(const struct spectrum* s, int comp = 0) :
        m_spectrum(s), m_comp(comp), m_index(0) {}

    void rewind(unsigned path_id) {
        m_index = 0;
    }

    unsigned vertex(double* x, double* y) {
        if(m_index >= spectra_points(m_spectrum)) {
            return agg::path_cmd_stop;
        }

        const float *val = spectra_get_values(m_spectrum, m_index);
        *x = double(val[0]);
        *y = double(val[1 + m_comp]);

        return (m_index++ == 0 ? agg::path_cmd_move_to : agg::path_cmd_line_to);
    }

private:
    const struct spectrum* m_spectrum;
    int m_comp;
    int m_index;
};

typedef vs_scaling_gen<spectrum_base_vs> spectrum_vs;

#endif
