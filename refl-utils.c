
/*
  $Id: refl-utils.c,v 1.4 2006/12/29 17:47:02 francesco Exp $
 */

#include "refl-utils.h"
#include "error-messages.h"
#include "data-table.h"


struct spectrum *
load_refl_data (const char *filename)
{
  struct spectrum *s;
  struct data_table *table;
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
  str_getline (ln, f);

  s = emalloc(sizeof(struct spectrum));

  s->config.system = SYSTEM_REFLECTOMETER;

  table = data_table_read_lines (f, "%*s %f %*f %f %*f\n", 0, 2);

  if (table == NULL)
    goto invalid_s;

  data_view_init (s->table, table);

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
