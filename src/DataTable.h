#ifndef DATA_TABLE_H
#define DATA_TABLE_H

#include <memory>
#include <gsl/gsl_matrix.h>

#include "Writer.h"
#include "Lexer.h"

class DataTable {
public:
	DataTable(int rows, int columns):
	m_data(new float[rows * columns]),
	m_view(gsl_matrix_float_view_array(m_data.get(), rows, columns))
	{ }

	const float& get(int i, int j) const {
		const gsl_matrix_float *m = &m_view.matrix;
		return m->data[i * m->tda + j];
	}

	float& get(int i, int j) {
		gsl_matrix_float *m = &m_view.matrix;
		return m->data[i * m->tda + j];
	}

    const float& operator()(int i, int j) const { return get(i, j); }
          float& operator()(int i, int j)       { return get(i, j); }

    int rows() const { return int(m_view.matrix.size1); }
    int cols() const { return int(m_view.matrix.size2); }

    int write(Writer& w);
    static std::unique_ptr<DataTable> read(Lexer& lexer);

private:
	std::unique_ptr<float[]> m_data;
	gsl_matrix_float_view m_view;
};

#endif
