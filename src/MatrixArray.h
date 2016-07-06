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

    int write(Writer& w, enum matrix_array_mode mode = MATRIX_LAYOUT_NORMAL) const;
    static std::unique_ptr<MatrixArray> read(Lexer& l, enum matrix_array_mode mode);

    const T& operator()(int i, int j) const { return m_data[Layout::index(m_rows, m_cols, i, j)]; }
          T& operator()(int i, int j)       { return m_data[Layout::index(m_rows, m_cols, i, j)]; }
          T         get(int i, int j) const { return m_data[Layout::index(m_rows, m_cols, i, j)]; }

    int rows() const { return m_rows; }
    int cols() const { return m_cols; }

    const T* data(int i, int j) const { return &m_data[Layout::index(m_rows, m_cols, i, j)]; }
          T* data(int i, int j)       { return &m_data[Layout::index(m_rows, m_cols, i, j)]; }

private:
    int m_rows, m_cols;
    std::unique_ptr<T[]> m_data;
};

template <typename T, typename L>
int MatrixArray<T, L>::write(Writer& w, enum matrix_array_mode mode) const {
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

template <typename T, typename L>
std::unique_ptr<MatrixArray<T, L> > MatrixArray<T, L>::read(Lexer& lexer, enum matrix_array_mode mode) {
    typedef MatrixArray<T, L> matrix_type;
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
                (*m)(i, j) = x;
            } else {
                (*m)(j, i) = x;
            }
        }
    }
    return m;
}

template <typename T, typename L>
Writer& operator<<(Writer& w, const MatrixArray<T, L>& m) {
    m.write(w);
    return w;
}

#endif
