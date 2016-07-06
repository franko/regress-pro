#ifndef INTERPOLATION_TABLE_H
#define INTERPOLATION_TABLE_H

#include <gsl/gsl_interp.h>

#include "cmpl.h"
#include "MatrixArray.h"
#include "Writer.h"

using std::shared_ptr;

class InterpolationTable {
public:
    using matrix_type = MatrixArray<double, layout_col_major>;

    InterpolationTable(): m_interp_n(nullptr) { }

    InterpolationTable(const shared_ptr<matrix_type>& table): m_table(table) {
        init_interp();
        prepare_interp();
    }

    InterpolationTable(const InterpolationTable& that): m_table(that.m_table) {
        init_interp();
        prepare_interp();
    }

    InterpolationTable(InterpolationTable&& that): m_table(std::move(that.m_table)) {
        m_interp_n = that.m_interp_n;
        m_interp_k = that.m_interp_k;
        m_accel = that.m_accel;
        m_interp_n = nullptr;
    }

    ~InterpolationTable() {
        if (is_interp_not_null()) {
            free_interp();
        }
    }

    complex interpolate(double wavelength) const;
    int length() const { return m_table->rows(); }
    const matrix_type& table() const { return *m_table; }

private:
    void init_interp();
    void free_interp();
    void prepare_interp();

    bool is_interp_not_null() const { return m_interp_n != nullptr; }

    shared_ptr<matrix_type> m_table;
    gsl_interp *m_interp_n, *m_interp_k;
    gsl_interp_accel *m_accel;

    friend Writer& operator<<(Writer& w, const InterpolationTable& interp_table);
};

#endif
