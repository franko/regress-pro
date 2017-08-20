#ifndef DISP_SAMPLE_TABLE_H
#define DISP_SAMPLE_TABLE_H

#include <gsl/gsl_interp.h>

#include "cmpl.h"
#include "rc_matrix.h"

struct disp_struct;

struct disp_sample_table {
    int len;
    rc_matrix *table;
    gsl_interp *interp_n, *interp_k;
    gsl_interp_accel *accel;
};

extern struct disp_class disp_sample_table_class;

extern void disp_sample_table_init(struct disp_sample_table *dt, int len);
extern void disp_sample_table_prepare_interp(struct disp_sample_table *dt);

extern struct disp_struct *
disp_sample_table_new_from_mat_file(const char * filename, str_ptr *error_msg);

extern struct disp_struct *
disp_sample_table_new_from_txt_file(const char * filename, int nkf_headers, str_ptr *error_msg);

extern void disp_sample_table_get_sample(const struct disp_sample_table *dt, int index, double *w, double *n, double *k);

#endif
