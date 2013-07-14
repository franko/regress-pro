/*
  $Id$
*/

#ifndef DISP_STABLE_H
#define DISP_STABLE_H

#include "cmpl.h"
#include "data-table.h"

__BEGIN_DECLS

struct disp_sampled_table {
    int points_number;

    float lambda_min, lambda_max;

    struct data_table *table_ref;
};

/* HO dispersion class */
extern struct disp_class disp_sampled_table_class;

void         disp_table_set_index_value(struct disp_table dt[], int idx,
                                        float nr, float ni);

void         disp_table_set_range(struct disp_table dt[],
                                  double lmin, double lmax);

void         disp_table_get_range(const struct disp_table dt[],
                                  double *lmin, double *lmax,
                                  int *points);

cmpl         disp_table_get_value_at_index(const struct disp_table dt[], int idx);

double       disp_table_get_lambda(const struct disp_table dt[], int idx);

extern struct disp_struct * disp_table_new_from_nk_file(const char * filename);

__END_DECLS

#endif
