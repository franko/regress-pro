/*
  $Id$
*/

#ifndef DISP_TABLE_H
#define DISP_TABLE_H

#include "cmpl.h"
#include "data-table.h"

__BEGIN_DECLS

struct disp_struct;

struct disp_table {
  int points_number;

  float lambda_min, lambda_max;
  float lambda_stride;

  struct data_table *table_ref;
};

extern struct disp_class disp_table_class;

extern struct disp_struct * disp_table_new_from_nk_file (const char * filename);

__END_DECLS

#endif
