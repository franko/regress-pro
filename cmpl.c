/*
  $Id: cmpl.c,v 1.5 2006/07/12 22:48:54 francesco Exp $
 */

#include <assert.h>
#include "common.h"
#include "cmpl.h"

cmpl_vector *
cmpl_vector_alloc (int nb)
{
  cmpl_vector *v = emalloc (sizeof(cmpl_vector));

  v->data = (cmpl *) emalloc (nb * sizeof(cmpl));
  v->size = nb;
  v->owner = 1;

  return v;
}

void
cmpl_vector_free (cmpl_vector *v)
{
  assert (v != NULL);
  if (v->data && v->owner)
    free (v->data);
  free (v);
}

void
cmpl_vector_set (cmpl_vector *v, int idx, cmpl val)
{
  if (idx < v->size)
    v->data[idx] = val;
  else
    assert (0);
}

cmpl
cmpl_vector_get (cmpl_vector *v, int idx)
{
  cmpl val;

  if (idx < v->size)
    val = v->data[idx];
  else
    assert (0);

  return val;
}
