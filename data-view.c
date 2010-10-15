/*
  $Id$
*/

#include <assert.h>
#include <string.h>

#include "data-view.h"


void
data_view_init (struct data_view *d, struct data_table *table)
{
  d->rows    = table->rows;
  d->columns = table->columns;

  d->idx_start = 0;
  d->idx_end   = table->rows;

  d->map = NULL;

  d->table = table;
}

void
data_view_copy (struct data_view *dest, const struct data_view *src)
{
  dest->rows    = src->rows;
  dest->columns = src->columns;

  dest->idx_start = src->idx_start;
  dest->idx_end   = src->idx_end;

  if (src->map)
    {
#warning potential memory leakage
      dest->map = emalloc (src->rows * sizeof(int));
      memcpy (dest->map, src->map, src->rows * sizeof(int));
    }
  else
    {
      dest->map = NULL;
    }

  dest->table = src->table;
  data_table_ref (src->table);
}

void
data_view_dealloc (struct data_view *d)
{
  if (d->map != NULL)
    free (d->map);

  data_table_unref (d->table);
  d->table = NULL;
}

void
data_view_init_empty (struct data_view *d)
{
  d->rows    = 0;
  d->columns = 0;

  d->idx_start = 0;
  d->idx_end   = 0;

  d->map = NULL;
  d->table = empty_data_table;
}

void
data_view_set_mask_offset (struct data_view *d, int start, int end)
{
  assert (d->map == NULL);

  assert (end <= d->table->rows && start >= 0 && start < end);

  d->idx_start = start;
  d->idx_end   = end;

  d->rows = end - start;
}

void
data_view_set_map (struct data_view *d, int size, int *map)
{
#ifndef NDEBUG
  int k;

  for (k = 0; k < size; k++)
    assert (map[k] < d->rows);
#endif

  if (d->map != NULL)
    free (d->map);

  d->map  = map;
  d->rows = size;
}

void
data_view_reset (struct data_view *d)
{
  if (d->map != NULL)
    {
      free (d->map);
      d->map = NULL;
    }

  d->idx_start = 0;
  d->idx_end   = d->table->rows;

  d->rows = d->table->rows;
}
