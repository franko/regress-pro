#ifndef DISP_SAMPLE_TABLE_H
#define DISP_SAMPLE_TABLE_H

#include "cmpl.h"
#include "data-table.h"

__BEGIN_DECLS

struct disp_struct;

struct disp_sample_table {
  int nb;
  struct data_table *table_ref;
};

/* HO dispersion class */
extern struct disp_class disp_sample_table_class;

extern struct disp_struct *
disp_sample_table_new_from_mat_file (const char * filename);

__END_DECLS

#endif
