
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "minsampling.h"
#include "data-view.h"


struct step_data {
  struct data_table *table;
  float epsilon;
  int size;
  int idx_max;
};


static int * minsampling_stepper (struct step_data *info, int idx, int idx_red);


int *
minsampling_stepper (struct step_data *info, int idx, int idx_red)
{
#define LAMBDA(idx) data_table_get(info->table, idx, 0)
#define ALPHA(idx)  data_table_get(info->table, idx, 1)
#define BETA(idx)   data_table_get(info->table, idx, 2)
  int *map = NULL;
  int j;

  for (j = idx + 2; j < info->idx_max; j++)
    {
      float pa = (ALPHA(j) - ALPHA(idx)) / (LAMBDA(j) - LAMBDA(idx));
      float pb = (BETA(j) -  BETA(idx) ) / (LAMBDA(j) - LAMBDA(idx));
      int k;

      for (k = idx+1; k < j; k++)
	{
	  float yint;
	  float dela, delb;

	  yint = ALPHA(idx) + (LAMBDA(k) - LAMBDA(idx)) * pa;
	  dela = yint - ALPHA(k);

	  yint = BETA(idx)  + (LAMBDA(k) - LAMBDA(idx)) * pb;
	  delb = yint - BETA(k);

	  if (dela > info->epsilon || dela < - info->epsilon)
	    break;
	  if (delb > info->epsilon || delb < - info->epsilon)
	    break;
	}

      if (k == j)
	continue;

      map = minsampling_stepper (info, j - 1, idx_red + 1);
      /* map cannot be null here */
      map[idx_red] = j - 1;
      return map;
    }

  info->size = idx_red + 1;
  
  map = emalloc((idx_red + 1) * sizeof(int));
  map[idx_red] = info->idx_max - 1;
  return map;
}
#undef ALPHA
#undef BETA
#undef LAMBDA

void
elliss_sample_minimize (struct spectrum *s, float dlmt)
{
  struct step_data info[1];
  int *map;

  info->idx_max = s->table->idx_end;
  info->table   = s->table->table;
  info->epsilon = dlmt;

  map = minsampling_stepper (info, s->table->idx_start, 1);
  map[0] = s->table->idx_start;

  data_view_set_map (s->table, info->size, map);
}
