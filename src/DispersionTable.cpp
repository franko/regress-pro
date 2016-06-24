#include "DispersionTable.h"

const DispersionTableClass dispersion_table_class{DISP_SAMPLE_TABLE, "Tabular", "table"};

DispersionTable::sample_type DispersionTable::sample(int i) const {
	double wavelength = m_table(i, 0);
	double n = m_table(i, 1), m_table(i, 2);
	return sample_type(wavelength, n, k);
}

complex DispersionTable::n_value(double wavelength) const {
    double n = gsl_interp_eval(m_interp_n, m_table.col_data(0), m_table.col_data(1), wavelength, m_accel);
    double k = gsl_interp_eval(m_interp_n, m_table.col_data(0), m_table.col_data(2), wavelength, m_accel);
    return complex(nx, -kx);
}

complex DispersionTable::n_value_deriv(double lam, complex der[]) const {
}

int DispersionTable::fp_number() const { return 0; }

int DispersionTable::write(Writer& w) const {

}
