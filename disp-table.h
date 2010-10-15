/*
  $Id$
*/

#ifndef DISP_TABLE_H
#define DISP_TABLE_H

#include "cmpl.h"
#include "data-table.h"

__BEGIN_DECLS

struct disp_table {
  int points_number;

  float lambda_min, lambda_max;
  float lambda_stride;

  struct data_table *table_ref;
};

/* HO dispersion class */
extern struct disp_class disp_table_class;

/*
void         disp_table_init_from_table (struct disp_table dt[],
					 struct data_table *table);

int          disp_table_load_nk_table   (const char * filename,
					 struct disp_table table[]);

cmpl         disp_table_get_n_value     (struct disp_table const dt[],
					 double lambda);

void         disp_table_update_range    (struct disp_table dt[]);

*/

void         disp_table_set_index_value (struct disp_table dt[], int idx,
					 float nr, float ni);

void         disp_table_set_range       (struct disp_table dt[],
					 double lmin, double lmax);

void         disp_table_get_range       (const struct disp_table dt[],
					 double *lmin, double *lmax,
					 int *points);

cmpl         disp_table_get_value_at_index (const struct disp_table dt[], int idx);

double       disp_table_get_lambda      (const struct disp_table dt[], int idx);

extern struct disp_struct * disp_table_new_from_nk_file (const char * filename);

__END_DECLS

#endif
