
/*
  $Id$
 */

#include <string.h>

#include "common.h"
#include "data-table.h"
#include "dispers.h"
#include "dispers-library.h"

struct disp_list_node {
  disp_t *content;
  const char *id;
  struct disp_list_node *next;
};

/* this structure mirror the struct data_table in data-table.h
   with the difference that a bigger array of floats is statically
   allocated. */
struct data_table_silicon {
  int rows;
  int columns;
  int ref_count;
  float heap[341 * 2];
};

static void add_dispersion_node (struct disp_list_node *curr,
				 struct disp_list_node *prev,
				 const char *id, disp_t *disp);

#include "si-paper-table.h"
#include "poly-table.h"

static struct disp_list_node *dispers_library_list;

void
add_dispersion_node (struct disp_list_node *curr, struct disp_list_node *prev,
		     const char *id, disp_t *disp)
{
  curr->content = disp;
  curr->id = id;
  curr->next = prev;
}

static disp_t *
make_poly_dispers ()
{
  disp_t *lookup_disp;
  const int nb_comp = 5;
  struct lookup_comp *comp_data = malloc (nb_comp * sizeof(struct lookup_comp));
  int j = 0;

  for (j = 0; j < nb_comp; j++)
    {
      struct lookup_comp *comp = comp_data + j;
      struct disp_table *dt;

      comp->p = poly_p_value[j];

      comp->disp = disp_new_with_name (DISP_TABLE, poly_comp_name[j]);
      dt = &comp->disp->disp.table;

      dt->points_number = 271;
      dt->lambda_min    = 240.0;
      dt->lambda_max    = 780.0;
      dt->lambda_stride = 2.0;
      dt->table_ref     = (struct data_table *) &poly_comp[j];
    }

  lookup_disp = disp_new_lookup ("Poly", 5, comp_data, 0.5);

  return lookup_disp;
}

void
dispers_library_init ()
{
#define NB_LIBRARY_DISPERS 5
  static struct disp_list_node node_prealloc[NB_LIBRARY_DISPERS];
  static struct ho_params sio2_ho_params[] = {
    {145.0, 15.78839, 0.0, 0.3333, 0.0}};
  struct disp_table *dt;
  double vac_cauchy_n[3] = {1, 0, 0};
  double vac_cauchy_k[3] = {0, 0, 0};
  double water_cauchy_n[3] = {1.31970012187958, 4677.3115234375, -108020320};
  double *water_cauchy_k = vac_cauchy_k;
  struct disp_list_node *node, *prev;
  disp_t *current;
  int idx = 0;

  prev = NULL;

  /* Silicon PAPER */
  current = disp_new_with_name (DISP_TABLE, "Si from paper");
  dt = & current->disp.table;

  dt->points_number = si_data_table.rows;
  dt->lambda_min    = SI_PAPER_WVLEN_MIN;
  dt->lambda_max    = SI_PAPER_WVLEN_MAX;
  dt->lambda_stride = SI_PAPER_WVLEN_STRIDE;
  dt->table_ref     = (struct data_table *) & si_data_table;

  node = node_prealloc + idx;
  add_dispersion_node (node, prev, "Si", current);
  prev = node;
  idx ++;

  /* Vacuum dispersion */
  current = disp_new_cauchy ("vacuum", vac_cauchy_n, vac_cauchy_k);

  node = node_prealloc + idx;
  add_dispersion_node (node, prev, "vacuum", current);
  prev = node;
  idx ++;

  /* water dispersion */
  current = disp_new_cauchy ("H2O", water_cauchy_n, water_cauchy_k);

  node = node_prealloc + idx;
  add_dispersion_node (node, prev, "H2O", current);
  prev = node;
  idx ++;

  current = disp_new_ho ("Thermal SiO2", 1, sio2_ho_params);

  node = node_prealloc + idx;
  add_dispersion_node (node, prev, "SiO2", current);
  prev = node;
  idx ++;

  current = make_poly_dispers ();

  node = node_prealloc + idx;
  add_dispersion_node (node, prev, "Poly", current);
  prev = node;
  idx ++;

  dispers_library_list = node;
}

disp_t *
dispers_library_search (const char *id)
{
  struct disp_list_node *node;
  disp_t *result = NULL;

  for (node = dispers_library_list; node; node = node->next)
    {
      if (strcasecmp (node->id, id) == 0)
	{
	  result = disp_copy (node->content);
	  break;
	}
    }

  return result;
}
