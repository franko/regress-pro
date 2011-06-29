/*
 $Id: fit-params.c,v 1.6 2006/07/12 22:48:54 francesco Exp $
 */

#include <stdio.h>
#include <string.h>
#include "fit-params.h"
#include "dispers.h"

static void get_disp_param_name (fit_param_t *fp, str_ptr buf);


void
set_model_param (fit_param_t *fpres, int lyr, enum disp_model_id model_id,
		 int param_nb)
{
  fpres->id = PID_LAYER_N;
  fpres->layer_nb = lyr;
  fpres->model_id = model_id;
  fpres->param_nb = param_nb;
}

void
set_thick_param (fit_param_t *fpres, int lyr)
{
  fpres->id = PID_THICKNESS;
  fpres->layer_nb = lyr;
  fpres->model_id = 0;
  fpres->param_nb = 0;
}

void
get_param_name (fit_param_t *fp, str_t name)
{
  switch (fp->id)
    {
    case PID_THICKNESS:
      str_printf(name, "T%i", fp->layer_nb);
      break;
    case PID_FIRSTMUL:
      str_printf(name, "1stmult");
      break;
    case PID_LAYER_N:
      get_disp_param_name (fp, name);
      break;
    default:
      str_printf(name, "###");
    }
}

void
get_disp_param_name (fit_param_t *fp, str_ptr buf)
{
  struct disp_class *dclass;
  void *iter;

  if (fp->id != PID_LAYER_N)
    {
      str_copy_c (buf, "###");
      return;
    }

  for (iter = disp_class_next (NULL); iter; iter =  disp_class_next (iter))
    {
      dclass = disp_class_from_iter (iter);
      if (dclass->model_id == fp->model_id)
	break;
    }

  if (iter)
    dclass->encode_param (buf, fp);
  else
    str_copy_c (buf, "###");
}

int
parse_fit_string (const char *s, seed_t *seed)
{
  int n;

  n = sscanf (s, "%lf-%lf,%lf", & seed->min, & seed->max, & seed->step);

  if (n == 3)
    {
      seed->type = SEED_RANGE;
      seed->seed = (seed->min + seed->max) / 2.0;
      return 0;
    }

  return 1;
}

struct fit_parameters *
fit_parameters_new (void)
{
  return (struct fit_parameters *) ARRAY_NEW(fit_param_t);
}

void
fit_parameters_free (struct fit_parameters *s)
{
  free (s->values);
  free (s);
}

void
fit_parameters_clear (struct fit_parameters *s)
{
  s->number = 0;
}

void
fit_parameters_add (struct fit_parameters *lst, fit_param_t const * fp)
{
  size_t idx = lst->number;

  ARRAY_CHECK_ALLOC(lst, fit_param_t, idx);

  memcpy (lst->values + idx, fp, sizeof(fit_param_t));

  lst->number ++;
}

struct seeds *
seed_list_new (void)
{
  return (struct seeds *) ARRAY_NEW(seed_t);
}

void
seed_list_free (struct seeds *s)
{
  free (s->values);
  free (s);
}

void
seed_list_add_simple (struct seeds *s, double v)
{
  size_t idx = s->number;

  ARRAY_CHECK_ALLOC(s, seed_t, idx);

  s->values[idx].type = SEED_SIMPLE;
  s->values[idx].seed = v;

  s->number ++;
}

void
seed_list_add (struct seeds *s, seed_t *v)
{
  size_t idx = s->number;
  ARRAY_CHECK_ALLOC(s, seed_t, idx);
  memcpy (s->values + idx, v, sizeof(seed_t));
  s->number ++;
}

struct strategy *
strategy_new ()
{
  struct strategy *r;

  r = emalloc (sizeof(struct strategy));

  r->parameters = fit_parameters_new ();
  r->seeds = seed_list_new ();

  return r;
}

void
strategy_free (struct strategy *s)
{
  fit_parameters_free (s->parameters);
  seed_list_free (s->seeds);
  free (s);
}

int
fit_parameters_are_RI_fixed (struct fit_parameters *f)
{
  int j;

  for (j = 0; j < f->number; j++)
    if (f->values[j].id == PID_LAYER_N)
      break;

  return (j >= f->number);
}

