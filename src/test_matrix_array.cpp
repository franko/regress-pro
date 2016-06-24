#include <cmath>

#include "str-util.h"
#include "MatrixArray.h"

typedef MatrixArray<double> matrix_array;

int main() {
    matrix_array m(10, 5);
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            m(i, j) = std::pow(double(i) / 10.0, j+1);
        }
    }
    Writer w;
    w << m;
    w.save_tofile("test.txt");

    str text;
    str_loadfile("test.txt", &text);
    
    Lexer lexer(text.text());
    auto mx = matrix_array::read(lexer, MATRIX_LAYOUT_TRANSPOSED);

    if (mx) {
        const matrix_array& m = *mx.get();
        for (int i = 0; i < m.rows(); i++) {
            for (int j = 0; j < m.cols(); j++) {
                printf("%d %d: %g\n", i, j, m(i, j));
            }
        }
        return 0;
    }

    return 1;
}