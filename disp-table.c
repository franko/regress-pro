/*
  $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dispers.h"
#include "disp-table.h"
#include "error-messages.h"

static void     disp_table_free      (struct disp_struct *d);
static disp_t * disp_table_copy      (const disp_t *d);

static cmpl     disp_table_n_value   (const disp_t *disp, double lam);

struct disp_class disp_table_class = {
  .disp_class_id       = DISP_TABLE,
  .model_id            = MODEL_NONE,

  .short_id            = "Table",
  .full_id             = "TableDispersion",

  .free                = disp_table_free,
  .copy                = disp_table_copy,

  .n_value             = disp_table_n_value,
  .fp_number           = disp_base_fp_number,
  .n_value_deriv       = NULL,
  .apply_param         = NULL,

  .decode_param_string = disp_base_decode_param_string,
  .encode_param        = NULL,
};

static void disp_table_init (struct disp_table dt[], int points);

void
disp_table_init (struct disp_table dt[], int points)
{
  dt->points_number = points;
  dt->lambda_min = 0.0;
  dt->lambda_max = 0.0;

  dt->table_ref = data_table_new (points, 2 /* columns */);
}

void
disp_table_free (struct disp_struct *d)
{
  data_table_unref (d->disp.table.table_ref);
  free (d);
}

disp_t *
disp_table_copy (const disp_t *src)
{
  disp_t *res = disp_base_copy (src);
  data_table_ref (res->disp.table.table_ref);
  return res;
}

static void
set_index_value (struct disp_table dt[], int idx,
			    float nr, float ni)
{
  data_table_set (dt->table_ref, idx, 0, nr);
  data_table_set (dt->table_ref, idx, 1, ni);
}

static void
set_range (struct disp_table dt[], double lmin, double lmax)
{
  dt->lambda_min    = lmin;
  dt->lambda_max    = lmax;
  dt->lambda_stride = (lmax - lmin) / (dt->points_number - 1);
}

static void
get_range (struct disp_table const dt[],
	   double *lmin, double *lmax, int *points)
{
  *lmin   = dt->lambda_min;
  *lmax   = dt->lambda_max;
  *points = dt->points_number; 
}

static cmpl
get_value_at_index (struct disp_table const dt[], int idx)
{
  double nr, ni;

  nr = data_table_get (dt->table_ref, idx, 0);
  ni = data_table_get (dt->table_ref, idx, 1);

  return nr - I * ni;
}

cmpl
disp_table_n_value (const disp_t *disp, double lam)
{
  const struct disp_table *table = & disp->disp.table;
  int ni, nb;
  double lami;
  cmpl a, b, n;
  double lmin, lmax;
  double dlam;

  get_range (table, &lmin, &lmax, &nb);

  dlam = (lmax - lmin) / (nb - 1);
  ni = (int) ((lam - lmin) / dlam);

  ni = (ni < 0 ? 0 : (ni >= nb-1 ? nb-2 : ni));

  lami = lmin + ni * dlam;
    
  a = get_value_at_index (table, ni);
  b = get_value_at_index (table, ni+1);
  n = a + (b - a) * (lam - lami) / dlam;

  return n;
}

struct disp_struct *
disp_table_new_from_nk_file (const char * filename)
{
  struct disp_struct *disp;
  struct disp_table *table;
  FILE * f;
  int j, npt, nread;
  float wlmax, wlmin;

  disp = disp_new (DISP_TABLE);
  table = & disp->disp.table;

  f = fopen (filename, "r");

  if (f == NULL)
    {
      notify_error_msg (LOADING_FILE_ERROR, "Cannot open %s", filename);
      return NULL;
    }

  nread = fscanf(f, "%*i %f %f %i\n", & wlmin, & wlmax, &npt);
  if (nread < 3)
    {
      notify_error_msg (LOADING_FILE_ERROR, "File %s not in NK format",
			filename);
      return NULL;
    }

  disp_table_init (table, npt+1);

  for (j = 0; j <= npt; j++)
    {
      float nr, ni;

      nread = fscanf(f, "%f %f\n", & nr, & ni);
      if (nread < 2)
	{
	  notify_error_msg (LOADING_FILE_ERROR, "invalid format for nk table");
	  goto disp_nk_free;
	}

      set_index_value (table, j, nr, ni);
    }

  set_range (table, wlmin * 1.0E3, wlmax * 1.0E3);

  fclose (f);
  return disp;

 disp_nk_free:
  disp_table_free (disp);
  fclose (f);
  return NULL;
}
