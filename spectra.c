#include <assert.h>
#include <string.h>

#include "common.h"
#include "spectra.h"
#include "refl-utils.h"
#include "error-messages.h"
#include "data-table.h"
#include "str.h"


static struct spectrum * load_ellips_spectrum (const char *filename);


struct spectrum *
load_ellips_spectrum (const char *filename)
{
  struct spectrum *s;
  struct system_config *cfg;
  struct data_table *data_table;
  int iseof, nr;
  str_t ln;
  FILE *f;

  f = fopen (filename, "r");
  
  if (f == NULL)
    {
      notify_error_msg (LOADING_FILE_ERROR, "error loading spectra %s",
			filename);
      return NULL;
    }

  str_init (ln, 64);

  s = emalloc(sizeof(struct spectrum));
  
  cfg = & s->config;

  cfg->system   = SYSTEM_UNDEFINED;
  cfg->aoi      = 71.7;
  cfg->analyzer = 25.0;
  cfg->numap    = 0.0;

  str_getline (ln, f);
  if (strstr (CSTR(ln), "SE ALPHA BETA"))
    cfg->system = SYSTEM_ELLISS_AB;
  else if (strstr (CSTR(ln), "SE PSI DELTA"))
    cfg->system = SYSTEM_ELLISS_PSIDEL;
  else
    goto invalid_s;

  for (;;) {
    iseof = str_getline (ln, f);
    
    if (iseof != 0)
      break;

    nr = sscanf (CSTR(ln), "AOI %lf", & cfg->aoi);
    if (nr == 1)
      continue;

    nr = sscanf (CSTR(ln), "NA %lf", & cfg->numap);
    if (nr == 1)
      continue;

    nr = sscanf (CSTR(ln), "A %lf", & cfg->analyzer);
    if (nr == 1)
      continue;

    break;
  }

  /* the values get converted in radians */
  cfg->aoi      = DEGREE(cfg->aoi);
  cfg->analyzer = DEGREE(cfg->analyzer);

  do {
    float lambda, alpha, beta;

    nr = sscanf (CSTR(ln), "%*s %f %f %f\n", & lambda, & alpha, & beta);
    if (nr < 3)
      goto invalid_s;

    data_table = data_table_read_lines (f, "%*s %f %f %f\n", 1, 3);

    if (data_table == NULL)
      goto invalid_s;

    data_table_set (data_table, 0, 0, lambda);
    data_table_set (data_table, 0, 1, alpha);
    data_table_set (data_table, 0, 2, beta);
  } while (0);

  data_view_init (s->table, data_table);
  
  str_free (ln);
  fclose (f);

  return s;
 invalid_s:
  notify_error_msg (LOADING_FILE_ERROR, "format of spectra %s is incorrect",
		    filename);
  free (s);
  str_free (ln);
  fclose (f);
  return NULL;
}

float
get_lambda_by_index (struct spectrum *s, int idx)
{
  assert (idx >= 0 && idx < s->table->rows);
  return data_view_get (s->table, idx, 0);
}

void
spectra_free (struct spectrum *s)
{
  data_view_dealloc (s->table);
  free (s);
}

struct spectrum *
load_gener_spectrum (const char *filename)
{
  struct spectrum *spectr;
  str_t ln;
  FILE *f;
  
  f = fopen (filename, "r");

  if (f == NULL)
    {
      notify_error_msg (LOADING_FILE_ERROR, "error loading spectra file %s",
			filename);
      return NULL;
    }

  str_init (ln, 64);
  str_getline (ln, f);

  fclose (f);

  if (strstr (CSTR(ln), "SE ALPHA BETA") || strstr (CSTR(ln), "SE PSI DELTA"))
    spectr = load_ellips_spectrum (filename);
  else
    spectr = load_refl_data (filename);

  str_free (ln);

  return spectr;
}

void
spectr_cut_range (struct spectrum *s, float inf, float sup)
{
  int j, jmin = -1, npt = 0;

  data_view_reset (s->table);

  for (j = 0; j < spectra_points (s); j++)
    {
      float lam = get_lambda_by_index (s, j);

      if (lam < inf || lam > sup)
	continue;

      if (jmin < 0)
	jmin = j;

      npt++;
    }

  data_view_set_mask_offset (s->table, jmin, jmin + npt);
}

struct spectrum *
spectra_copy (struct spectrum *src)
{
  struct spectrum *copy = emalloc (sizeof(struct spectrum));
  copy->config = src->config;
  data_view_copy (copy->table, src->table);
  return copy;
}

struct spectrum *
spectra_alloc (struct spectrum *s)
{
  struct spectrum *synth = emalloc (sizeof(struct spectrum));
  int rows = spectra_points (s), cols = s->table->columns;
  struct data_table *table = data_table_new (rows, cols);
  synth->config = s->config;
  data_view_init (synth->table, table);
  return synth;
}

void
spectra_resize (struct spectrum *s, int nr)
{
  struct data_view *dv = s->table;
  data_view_reset (dv);
  if (nr <= dv->table->rows)
    {
      data_view_set_mask_offset (dv, 0, nr);
    }
  else
    {
      data_view_dealloc (dv);
      dv->rows  = nr;
      dv->table = data_table_new (nr, dv->columns);
    }
}

float const *
spectra_get_values (struct spectrum const *s, int idx)
{
  return data_view_get_row (s->table, idx);
}
