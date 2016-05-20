#ifndef MATRIX_ARRAY_H
#define MATRIX_ARRAY_H

#include <memory>

#include "defs.h"
#include "Writer.h"
#include "Lexer.h"

enum matrix_array_mode {
    MATRIX_LAYOUT_NORMAL,
    MATRIX_LAYOUT_TRANSPOSED,
};

template <typename T>
class MatrixArray {
public:
    MatrixArray(int rows, int cols):
    m_rows(rows), m_cols(cols), m_data(new double[rows * cols])
    { }

    int write(Writer& w, enum matrix_array_mode mode = MATRIX_LAYOUT_NORMAL);
    static std::unique_ptr<MatrixArray> read(Lexer& l, enum matrix_array_mode mode);

    const T& operator()(int i, int j) const { return m_data[i * m_cols + j]; }
          T& operator()(int i, int j)       { return m_data[i * m_cols + j]; }
          T         get(int i, int j) const { return m_data[i * m_cols + j]; }

    int rows() const { return m_rows; }
    int cols() const { return m_cols; }
    T *data() { return m_data.get(); }
private:
    int m_rows, m_cols;
    std::unique_ptr<T[]> m_data;
};

template <typename T>
int MatrixArray<T>::write(Writer& w, enum matrix_array_mode mode) {
    int rows = (mode == MATRIX_LAYOUT_NORMAL ? m_rows : m_cols);
    int cols = (mode == MATRIX_LAYOUT_NORMAL ? m_cols : m_rows);

    w << rows << cols;
    for (int i = 0; i < rows; i++) {
        w.newline();
        for (int j = 0; j < cols; j++) {
            double x = mode == MATRIX_LAYOUT_NORMAL ? get(i, j) : get(j, i);
            w << x;
        }
    }
    return 0;
}

template <typename T>
std::unique_ptr<MatrixArray<T> > MatrixArray<T>::read(Lexer& lexer, enum matrix_array_mode mode) {
    typedef MatrixArray<T> Matrix;
    int rows, cols;
    if (lexer.integer(&rows)) return nullptr;
    if (lexer.integer(&cols)) return nullptr;
    int array_rows = (mode == MATRIX_LAYOUT_NORMAL ? rows : cols);
    int array_cols = (mode == MATRIX_LAYOUT_NORMAL ? cols : rows);
    std::unique_ptr<Matrix> mref(new Matrix(array_rows, array_cols));
    Matrix& m = *mref.get();
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double x;
            if (lexer.number(&x)) return nullptr;
            if (mode == MATRIX_LAYOUT_NORMAL) {
                m(i, j) = x;
            } else {
                m(j, i) = x;
            }
        }
    }
    return mref;
}

template <typename T>
Writer& operator<<(Writer& w, MatrixArray<T>& m) {
    m.write(w);
    return w;
}

#endif
