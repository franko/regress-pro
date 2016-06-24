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

struct layout_row_major {
    static int index(int rows, int cols, int i, int j) { return cols * i + j; }
};

struct layout_col_major {
    static int index(int rows, int cols, int i, int j) { return rows * j + i; }
};

template <typename T, typename Layout>
class MatrixArray {
public:
    MatrixArray() { }
    MatrixArray(int rows, int cols):
    m_rows(rows), m_cols(cols), m_data(new T[rows * cols])
    { }

    void resize(int rows, int cols) {
        m_rows = rows;
        m_cols = cols;
        m_data.reset(new T[rows * cols]);
    }

    int write(Writer& w, enum matrix_array_mode mode = MATRIX_LAYOUT_NORMAL);
    static std::unique_ptr<MatrixArray> read(Lexer& l, enum matrix_array_mode mode);

    const T& operator()(int i, int j) const { return m_data[Layout::index(m_rows, m_cols, i, j)]; }
          T& operator()(int i, int j)       { return m_data[Layout::index(m_rows, m_cols, i, j)]; }
          T         get(int i, int j) const { return m_data[Layout::index(m_rows, m_cols, i, j)]; }

    int rows() const { return m_rows; }
    int cols() const { return m_cols; }

    const T* row_data(int i) const { return &m_data[Layout::index(m_rows, m_cols, i, 0)]; }
          T* row_data(int i)       { return &m_data[Layout::index(m_rows, m_cols, i, 0)]; }
    const T* col_data(int j) const { return &m_data[Layout::index(m_rows, m_cols, 0, j)]; }
          T* col_data(int j)       { return &m_data[Layout::index(m_rows, m_cols, 0, j)]; }

    const T* data() const { return m_data.get(); }
          T* data()       { return m_data.get(); }
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
            const T x = mode == MATRIX_LAYOUT_NORMAL ? get(i, j) : get(j, i);
            w << x;
        }
    }
    return 0;
}

template <typename T>
std::unique_ptr<MatrixArray<T> > MatrixArray<T>::read(Lexer& lexer, enum matrix_array_mode mode) {
    typedef MatrixArray<T> matrix_type;
    int rows, cols;
    lexer >> rows >> cols;
    int array_rows = (mode == MATRIX_LAYOUT_NORMAL ? rows : cols);
    int array_cols = (mode == MATRIX_LAYOUT_NORMAL ? cols : rows);
    std::unique_ptr<matrix_type> m(new matrix_type(array_rows, array_cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            T x;
            lexer >> x;
            if (mode == MATRIX_LAYOUT_NORMAL) {
                *m(i, j) = x;
            } else {
                *m(j, i) = x;
            }
        }
    }
    return m;
}

template <typename T>
Writer& operator<<(Writer& w, MatrixArray<T>& m) {
    m.write(w);
    return w;
}

#endif
