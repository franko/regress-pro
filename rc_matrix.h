#ifndef RC_MATRIX_H
#define RC_MATRIX_H

#include <gsl/gsl_matrix.h>

#include "defs.h"
#include "writer.h"
#include "lexer.h"

__BEGIN_DECLS

struct _rc_matrix {
    gsl_matrix_view view;
    int ref_count;
    double data[1];
};

enum rc_matrix_mode {
    RC_MATRIX_NORMAL,
    RC_MATRIX_TRANSPOSED,
};

typedef struct _rc_matrix rc_matrix;

extern rc_matrix *rc_matrix_alloc(size_t rows, size_t cols);
extern void rc_matrix_ref(rc_matrix *m);
extern void rc_matrix_unref(rc_matrix *m);
extern int rc_matrix_write(writer_t *w, const rc_matrix *m, enum rc_matrix_mode mode);
extern rc_matrix *rc_matrix_read(lexer_t *l, enum rc_matrix_mode mode);

__END_DECLS

#endif
