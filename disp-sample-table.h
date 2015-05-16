#ifndef DISP_SAMPLE_TABLE_H
#define DISP_SAMPLE_TABLE_H

#include <gsl/gsl_interp.h>

#include "cmpl.h"
#include "rc_matrix.h"

__BEGIN_DECLS

struct disp_struct;

struct disp_sample_table {
    int len;
    rc_matrix *table;
    gsl_interp *interp_n, *interp_k;
    gsl_interp_accel *accel;
};

extern struct disp_class disp_sample_table_class;

extern struct disp_struct *
disp_sample_table_new_from_mat_file(const char * filename, str_ptr *error_msg);

extern void disp_sample_table_get_sample(struct disp_sample_table *dt, int index, double *w, double *n, double *k);

__END_DECLS

#endif
