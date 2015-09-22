#include "common.h"
#include "rc_matrix.h"

rc_matrix *rc_matrix_alloc(size_t rows, size_t cols)
{
    rc_matrix *m = emalloc(sizeof(rc_matrix) + sizeof(double) * (rows * cols - 1));
    m->view = gsl_matrix_view_array(m->data, rows, cols);
    m->ref_count = 1;
    return m;
}

void rc_matrix_ref(rc_matrix *m)
{
    m->ref_count++;
}

void rc_matrix_unref(rc_matrix *m)
{
    m->ref_count--;
    if (m->ref_count <= 0) {
        free(m);
    }
}

int rc_matrix_write(writer_t *w, const rc_matrix *m, enum rc_matrix_mode mode)
{
    const gsl_matrix *mat = &m->view.matrix;
    int rows = (mode == RC_MATRIX_NORMAL ? mat->size1 : mat->size2);
    int cols = (mode == RC_MATRIX_NORMAL ? mat->size2 : mat->size1);
    writer_printf(w, "%d %d", rows, cols);
    writer_newline(w);
    int i, j;
    for (i = 0; i < rows; i++) {
        if (i > 0) {
            writer_newline(w);
        }
        for (j = 0; j < cols; j++) {
            if (j > 0) {
                writer_printf(w, " ");
            }
            double x = mode == RC_MATRIX_NORMAL ? gsl_matrix_get(mat, i, j) : gsl_matrix_get(mat, j, i);
            writer_printf(w, "%g", x);
        }
    }
    return 0;

}

rc_matrix *rc_matrix_read(lexer_t *l, enum rc_matrix_mode mode)
{
    int rows, cols;
    if (lexer_integer(l, &rows)) return NULL;
    if (lexer_integer(l, &cols)) return NULL;
    int i, j;
    rc_matrix *m;
    if (mode == RC_MATRIX_NORMAL) {
        m = rc_matrix_alloc(rows, cols);
    } else {
        m = rc_matrix_alloc(cols, rows);
    }
    gsl_matrix *mat = &m->view.matrix;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            double x;
            if (lexer_number(l, &x)) goto table_exit;
            if (mode == RC_MATRIX_NORMAL) {
                gsl_matrix_set(mat, i, j, x);
            } else {
                gsl_matrix_set(mat, j, i, x);
            }
        }
    }
    return m;
table_exit:
    rc_matrix_unref(m);
    return NULL;
}
