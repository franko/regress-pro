#include "DataTable.h"

int DataTable::write(Writer& w) {
    w << rows() << cols();
    for (int i = 0; i < rows(); i++) {
        w.newline();
        for (int j = 0; j < cols(); j++) {
            w << get(i, j);
        }
    }
    return 0;
}

std::unique_ptr<DataTable> DataTable::read(Lexer& lexer) {
    int rows, cols;
    if (lexer.integer(&rows)) return nullptr;
    if (lexer.integer(&cols)) return nullptr;
    std::unique_ptr<DataTable> table(new DataTable(rows, cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double x;
            if (lexer.number(&x)) return nullptr;
            (*table)(i, j) = float(x);
        }
    }
    return table;
}
