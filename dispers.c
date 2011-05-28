#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <gsl/gsl_vector.h>

#include "dispers.h"
#include "common.h"
#include "error-messages.h"
#include "cmpl.h"
#include "str.h"
#include "fit-params.h"

static int lookup_find_interval (struct disp_lookup const * lookup, double p);



disp_t *
load_nk_table (const char * filename)
{
  const char *bname;
  disp_t * disp;

  disp = disp_table_new_from_nk_file (filename);

  if (disp == NULL)
    return NULL;

  bname = strrchr (filename, '/');
  if (bname)
    bname++;

  str_copy_c (disp->name, bname);

  return disp;
}

disp_t *
load_mat_dispers (const char * filename)
{
  return disp_sample_table_new_from_mat_file (filename);
}

static int
lookup_find_interval (struct disp_lookup const * lookup, double p)
{
  int j;

  assert (lookup->nb_comps >= 2);

  for (j = 1; j < lookup->nb_comps - 1; j++)
    {
      if (lookup->component[j].p >= p)
	break;
    }

  return j-1;
}

complex double
n_value (const disp_t *disp, double lam)
{
  cmpl n;
  struct disp_lookup const * lookup;
  
  if (disp->dclass != NULL)
    {
      return disp->dclass->n_value(disp, lam);
    }
  
  switch (disp->type)
    {
      double p1, p2;
      cmpl n1, n2;
      int j;

    case DISP_LOOKUP:
      lookup = & disp->disp.lookup;

      j = lookup_find_interval (lookup, lookup->p);

      n1 = n_value (lookup->component[j].disp, lam);
      n2 = n_value (lookup->component[j+1].disp, lam);
      p1 = lookup->component[j].p;
      p2 = lookup->component[j+1].p;
      n = n1 + (n2 - n1) * ((lookup->p - p1) / (p2 - p1));
      break;
    default:
      assert (0);
    }
        
  return n;
}

void
n_value_cpp (const disp_t *mat, double lam, double *nr, double *ni)
{
  cmpl n = n_value (mat, lam);
  *nr = creal(n);
  *ni = - cimag(n);
}

int
disp_get_number_of_params (disp_t *d)
{
  if (d->dclass != NULL)
    {
      return d->dclass->fp_number(d);
    }

  switch (d->type)
    {
    case DISP_LOOKUP:
      return 1;
    default:
      assert (0);
    }

  return 0;
}

int
dispers_apply_param (disp_t *d, const fit_param_t *fp, double val)
{
  int res = 1;

  if (fp->id != PID_LAYER_N)
    return res;

  if (d->dclass != NULL)
    {
      return d->dclass->apply_param(d, fp, val);
    }

  switch (d->type)
    {
    case DISP_LOOKUP:
      if (fp->model_id == MODEL_LOOKUP)
	d->disp.lookup.p = val;
      break;
    default:
      /* */ ;
    }

  return res;
}

void
get_model_param_deriv (const disp_t *disp, struct deriv_info *deriv_info,
		       const fit_param_t *fp, double lambda,
		       double *dnr, double *dni)
{
  enum disp_type disp_type;
  const struct disp_lookup * lookup;
  cmpl val = 0.0 + I * 0.0;

  if (disp->dclass != NULL)
    {
      if (! deriv_info->is_valid)
        {
	  int nb = disp->dclass->fp_number(disp);
	  deriv_info->val = cmpl_vector_alloc (nb);
	  disp->dclass->n_value_deriv(disp, lambda, deriv_info->val);
	  deriv_info->is_valid = 1;
	}

      val = cmpl_vector_get (deriv_info->val, fp->param_nb);

      *dnr = creal (val);
      *dni = cimag (val);

      return;      
    }

  disp_type = disp->type;

  switch (fp->model_id)
    {
      cmpl n2, n1;
      double p1, p2;
      int j;

    case MODEL_LOOKUP:
      if (disp_type != DISP_LOOKUP)
	break;
      lookup = & disp->disp.lookup;

      j  = lookup_find_interval (lookup, lookup->p);

      n1 = n_value (lookup->component[j].disp, lambda);
      n2 = n_value (lookup->component[j+1].disp, lambda);
      p1 = lookup->component[j].p;
      p2 = lookup->component[j+1].p;
      val = (n2 - n1) / (p2 - p1);
      break;
    default:
      assert (0);
    }
  
  *dnr = creal (val);
  *dni = cimag (val);
}

void
disp_free (disp_t *d)
{
  if (d->dclass != NULL)
    {
      d->dclass->free (d);
      return;
    }

  str_free (d->name);
  switch (d->type)
    {
      int j;
    case DISP_LOOKUP:
      for (j = 0; j < d->disp.lookup.nb_comps; j++)
	disp_free (d->disp.lookup.component[j].disp);
      free (d->disp.lookup.component);
      break;
    default:
      assert(0);
      /* */ ;
    }
  free (d);
}

disp_t *
disp_new (enum disp_type tp)
{
  return disp_new_with_name (tp, NULL);
}

disp_t *
disp_new_with_name (enum disp_type tp, const char *name)
{
  disp_t *d = emalloc (sizeof(disp_t));
  d->dclass = disp_class_lookup (tp);
  d->type = tp;
  if (name)
    str_init_from_c (d->name, name);
  else
    str_init (d->name, 16);
  return d;
}

disp_t *
disp_base_copy (const disp_t *src)
{
  disp_t *res = emalloc (sizeof(disp_t));
  memcpy (res, src, sizeof(disp_t));
  str_init_from_str (res->name, src->name);
  return res;
}

int
disp_base_decode_param_string (const char *param)
{
  return -1;
}

int
disp_base_fp_number (const disp_t *src)
{
  return 0;
}

disp_t *
disp_copy (disp_t *d)
{
  disp_t *res;

  if (d->dclass != NULL)
    {
      return d->dclass->copy (d);
    }

  res = emalloc (sizeof(disp_t));
  memcpy (res, d, sizeof(disp_t));
  str_init_from_str (res->name, d->name);

  switch (d->type)
    {
      struct lookup_comp *newcomp;
      int sz, j;

    case DISP_LOOKUP:
      sz = res->disp.lookup.nb_comps * sizeof(struct lookup_comp);
      newcomp = emalloc (sz);
      memcpy (newcomp, res->disp.lookup.component, sz);
      for (j = 0; j < res->disp.lookup.nb_comps; j++)
	newcomp[j].disp = disp_copy (newcomp[j].disp);
      res->disp.lookup.component = newcomp;
      break;
    default:
      notify_error_msg (LOADING_FILE_ERROR, "material card corrupted.\n");
    }

  return res;
}

disp_t *
disp_new_lookup (const char *name, int nb_comps, struct lookup_comp *comp,
		 double p0)
{
  disp_t *d = disp_new_with_name (DISP_LOOKUP, name);

  d->disp.lookup.p = p0;
  d->disp.lookup.nb_comps = nb_comps;
  d->disp.lookup.component = comp;

  return d;
}

int
disp_integrity_check (disp_t *d)
{
  int tp; 
  if (d == NULL)
    return 1;
  tp = d->type;
  return (tp <= DISP_UNSET || tp >= DISP_END_OF_TABLE);
}

int
disp_check_fit_param (disp_t *d, fit_param_t *fp)
{
  int ok = 0;

  if (d->dclass != NULL)
    {
      int nb = d->dclass->fp_number (d);
      if (fp->id != PID_LAYER_N)
	return 1;
      if (d->dclass->model_id != fp->model_id)
	return 1;
      if (fp->param_nb < 0 || fp->param_nb >= nb)
	return 1;
      return 0;
    }

  switch (d->type)
    {
    case DISP_LOOKUP:
      ok = (fp->id == PID_LAYER_N && fp->model_id == MODEL_LOOKUP);
      ok = ok && fp->param_nb == 0;
      break;
    default:
      /* */ ;
    }

  return (ok ? 0 : 1);
}

static int
lookup_decode (fit_param_t *fp, size_t layer, const char *s)
{
  if (strcmp (s, "P") != 0)
    return 1;

  set_model_param (fp, layer, MODEL_LOOKUP, 0);

  return 0;
}

int
decode_fit_param (fit_param_t *fp, const str_t str)
{
  struct disp_class_node *disp_node;
  const char *s = CSTR(str);
  const char *snext;
  int layer;

  if (strcmp (s, "RMult") == 0)
    {
      fp->id = PID_FIRSTMUL;
      return 0;
    }
  
  snext = strchr (s, (int) ':');
  if (snext == NULL)
    return 1;
  if (strncmp (s, "Th:", 3) == 0)
    {
      char *tail;
      s = snext + 1;
      layer = strtol (s, & tail, 10);
      if (*tail != 0 || tail == s || layer < 0)
	return 1;
      set_thick_param (fp, layer);
      return 0;
    }
  else if (strncmp (s, "RI:", 3) == 0)
    {
      char *tail;
      s = snext + 1;
      layer = strtol (s, & tail, 10);
      if (*tail != ':' || tail == s || layer < 0)
	return 1;
      s = tail + 1;
      snext = strchr (s, (int) ':');
      if (snext == NULL)
	return 1;

      for (disp_node = disp_class_list; disp_node; disp_node = disp_node->next)
	{
	  struct disp_class *dclass = disp_node->value;
	  size_t dclass_len = strlen (dclass->short_id);

	  if (strncmp (s, dclass->short_id, dclass_len) == 0)
	    {
	      int fp_number;
	      const char *currs = s + dclass_len;
	      if (currs[0] != ':')
		continue;
	      currs++;
	      fp_number = dclass->decode_param_string (currs);
	      if (fp_number < 0)
		return 1;
	      set_model_param (fp, layer, dclass->model_id, fp_number);
	      return 0;
	    }
	}
    
      if (strncmp (s, "Lookup:", 7) == 0)
	return lookup_decode (fp, layer, snext+1);
    }

  return 1;
}
