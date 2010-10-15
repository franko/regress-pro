/*
  $Id: stack.c,v 1.6 2006/07/12 22:48:55 francesco Exp $
 */

#include <string.h>
#include "stack.h"
#include "str.h"

void
stack_init_raw (stack_t *s, size_t nb_init)
{
  s->nb = 0;
  s->nb_alloc = nb_init;
  s->disp = emalloc (nb_init * sizeof(void *));
  /* For thicknesses we allocate more space of what is actually needed. */
  s->thickness = emalloc (nb_init * sizeof(double));
}

void
stack_init (stack_t *s)
{
  stack_init_raw (s, 8);
}

void
stack_add_layer (stack_t *s, disp_t *lyr, double th)
{
  size_t idx;

  if ((size_t)s->nb >= s->nb_alloc)
    {
      size_t nsz;

      s->nb_alloc *= 2;
      nsz = s->nb_alloc;

      s->disp = erealloc (s->disp, nsz * sizeof(void *));

      /* For thicknesses we allocate more space of what is actually needed. */
      s->thickness = erealloc (s->thickness,  nsz * sizeof(double));
    }

  idx = s->nb;

  s->disp[idx] = lyr;
  if (idx > 0)
    s->thickness[idx - 1] = th;

  s->nb ++;
}

stack_t *
stack_copy (stack_t *s)
{
  int j;
  stack_t *r = emalloc (sizeof(stack_t));
  memcpy (r, s, sizeof(stack_t));

  r->nb_alloc = s->nb;

  r->thickness = emalloc (r->nb_alloc * sizeof(double));
  memcpy (r->thickness, s->thickness, r->nb_alloc * sizeof(double));
  
  r->disp = emalloc (r->nb_alloc * sizeof(disp_t *));
  for (j = 0; j < r->nb; j++)
    r->disp[j] = disp_copy (s->disp[j]);

  return r;
}

int
stack_apply_param (stack_t *s, const fit_param_t *fp, double val)
{
  int nlyr;
  int res = 0;

  switch (fp->id)
    {
    case PID_THICKNESS:
      nlyr = fp->layer_nb;
      s->thickness[ nlyr - 1 ] = val;
      break;
    case PID_LAYER_N:
      nlyr = fp->layer_nb;
      dispers_apply_param (s->disp[nlyr], fp, val);
      break;
    default:
      res = 1;
    }

  return res;
}

const double *
stack_get_ths_list (const stack_t *s)
{
  return s->thickness;
}

void
stack_get_ns_list (stack_t *s, cmpl *ns, double lambda)
{
  int j;
  for (j = 0; j < s->nb; j++)
    ns[j] = n_value (s->disp[j], lambda);
}

void
stack_free (stack_t *s)
{
  int j;
  free (s->thickness);
  for (j = 0; j < s->nb; j++)
    disp_free (s->disp[j]);
  free (s->disp);
  free (s);
}
