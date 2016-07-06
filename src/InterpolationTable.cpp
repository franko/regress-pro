#include "InterpolationTable.h"

void InterpolationTable::init_interp() {
    const int data_length = m_table->rows();
    m_interp_n = gsl_interp_alloc(gsl_interp_cspline, data_length);
    m_interp_k = gsl_interp_alloc(gsl_interp_cspline, data_length);
    m_accel = gsl_interp_accel_alloc();
}

void InterpolationTable::free_interp() {
    gsl_interp_accel_free(m_accel);
    gsl_interp_free(m_interp_n);
    gsl_interp_free(m_interp_k);
}

void InterpolationTable::prepare_interp() {
    const int len = m_table->rows();
    gsl_interp_init(m_interp_n, m_table->data(0, 0), m_table->data(0, 1), len);
    gsl_interp_init(m_interp_k, m_table->data(0, 0), m_table->data(0, 2), len);
}

complex InterpolationTable::interpolate(double wavelength) const {
    const int len = m_table->rows();
    double n, k;
    if (wavelength <= m_table->get(0, 0)) {
        n = m_table->get(0, 1);
        k = m_table->get(0, 2);
    } else if (wavelength >= m_table->get(len - 1, 0)) {
        n = m_table->get(len - 1, 1);
        k = m_table->get(len - 1, 2);
    } else {
        n = gsl_interp_eval(m_interp_n, m_table->data(0, 0), m_table->data(0, 1), wavelength, m_accel);
        k = gsl_interp_eval(m_interp_k, m_table->data(0, 0), m_table->data(0, 2), wavelength, m_accel);
    }
    return complex(n, -k);
}

Writer& operator<<(Writer& w, const InterpolationTable& interp) {
    w << *(interp.m_table);
    return w;
}
