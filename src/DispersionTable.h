#ifndef DISPERSION_TABLE_H
#define DISPERSION_TABLE_H

#include "dispers.h"
#include "InterpolationTable.h"

using std::unique_ptr;
using std::shared_ptr;

struct DispersionTableClass : DispersionClass {
    using DispersionClass::DispersionClass;
    unique_ptr<Dispersion> read(Lexer& lexer) const override;
};

extern const DispersionTableClass dispersion_table_class;

class DispersionTable : public Dispersion {
public:
    using matrix_type = MatrixArray<double, layout_col_major>;

    DispersionTable(const str& name, const shared_ptr<matrix_type>& table)
    : Dispersion(name, dispersion_table_class), m_interp_table(table)
    { }

    complex n_value(double lambda) const override;
    complex n_value_deriv(double lam, complex der[]) const override;
    int fp_number() const override;
    int write(Writer& w) const override;

    static unique_ptr<DispersionTable> load_mat_file(const char * filename);

private:
    InterpolationTable m_interp_table;
};

#endif
