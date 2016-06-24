#ifndef DISPERSION_TABLE_H
#define DISPERSION_TABLE_H

#include <tuple>

#include <gsl/gsl_interp.h>

#include "dispers.h"
#include "MatrixArray.h"

using std::unique_ptr;

struct DispersionTableClass : DispersionClass {
	using DispersionClass::DispersionClass;
	unique_ptr<Dispersion> read(Lexer& lexer) const override;
};

extern const DispersionTableClass dispersion_table_class;

class DispersionTable : public Dispersion {
public:
	using sample_type = std::tuple<double, double, double>;
	using matrix_type = MatrixArray<double, layout_col_major>;

	DispersionTable(const char *name, matrix_type&& table)
	: Dispersion(name, dispersion_table_class), m_table(table) {
		init_interp();
	}

	~DispersionTable() {
		free_interp();
	}

	sample_type sample(int index) const;

	complex n_value(double lambda) const override;
    complex n_value_deriv(double lam, complex der[]) const override;
    int fp_number() const override;
    int write(Writer& w) const override;

    static unique_ptr<DispersionTable> load_mat_file(const char * filename);

private:
	matrix_type m_table;
    gsl_interp *m_interp_n, *m_interp_k;
    gsl_interp_accel *m_accel;
};

#endif
