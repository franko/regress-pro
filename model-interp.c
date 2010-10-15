
/*
  $Id: model-interp.c,v 1.4 2006/05/12 15:03:15 af27 Exp $
 */

#include <string.h>
#include "model-interp.h"
#include "dispers.h"
#include "str-util.h"

static int ho_param_interp (struct gen_record *rec, struct ho_params *ho);


struct toplevel_obj *
cauchymodel_interp (struct symtab *symtab, struct gen_record *rec)
{
  disp_t *new_dispersion;
  struct toplevel_obj *res;
  struct value_lst * vlst;
  str_ptr name;
  double n[3], k[3];
  int j;

  if (rec->singleton)
    return NULL;
  name = get_named_field_as_string ("name", rec->cont.list);

  vlst = get_named_field_as_list ("n", rec->cont.list);
  for (j = 0; j < 3 && vlst; j++, vlst = vlst->next)
    {
      struct value *v = vlst->value;
      if (v->type != VAL_TP_DOUBLE)
	break;
      n[j] = v->cont.real;
    }
  if (j < 3 || vlst)
    return NULL;


  vlst = get_named_field_as_list ("k", rec->cont.list);
  for (j = 0; j < 3 && vlst; j++, vlst = vlst->next)
    {
      struct value *v = vlst->value;
      if (v->type != VAL_TP_DOUBLE)
	break;
      k[j] = v->cont.real;
    }
  if (j < 3 || vlst)
    return NULL;

  new_dispersion = disp_new_cauchy (name ? CSTR(name) : "", n, k);
  res = toplevel_obj_new (TL_TYPE_DISP, new_dispersion);

  return res;
}

int
ho_param_interp (struct gen_record *rec, struct ho_params *ho)
{
  int err = 0;

  if (strcmp (CSTR(rec->id), "HO") != 0)
    return 1;
  if (rec->singleton)
    return 1;

  err = err || get_named_field_as_double ("nosc", rec->cont.list, & ho->nosc); 
  err = err || get_named_field_as_double ("en",   rec->cont.list, & ho->en);
  err = err || get_named_field_as_double ("eg",   rec->cont.list, & ho->eg);
  err = err || get_named_field_as_double ("nu",   rec->cont.list, & ho->nu);
  err = err || get_named_field_as_double ("phi",  rec->cont.list, & ho->phi);

  return err;
}

struct toplevel_obj *
ho_model_interp (struct symtab *symtab, struct gen_record *rec)
{
  disp_t *new_dispersion;
  struct toplevel_obj *res;
  struct value_lst * plst;
  struct ho_params * hop;
  str_ptr name;
  int nb_hos, j;

  if (rec->singleton)
    return NULL;

  name = get_named_field_as_string ("name", rec->cont.list);

  plst = get_named_field_as_list ("parameters", rec->cont.list);
  if (plst == NULL)
    return NULL;

  nb_hos = valuelst_elements_nb (plst);
  if (nb_hos < 1)
    return NULL;

  hop = emalloc (nb_hos * sizeof(struct ho_params));
  
  for (j = 0; j < nb_hos && plst; j++, plst = plst->next)
    {
      struct gen_record *subrec;

      if (plst->value->type != VAL_TP_RECORD)
	break;

      subrec = plst->value->cont.record;
      if (ho_param_interp (subrec, hop + j))
	break;
    }

  if (j < nb_hos)
    {
      free (hop);
      return NULL;
    }

  new_dispersion = disp_new_ho (name ? CSTR(name) : "", nb_hos, hop);
  res = toplevel_obj_new (TL_TYPE_DISP, new_dispersion);

  free (hop);
  
  return res;
}


struct toplevel_obj *
bruggeman_model_interp (struct symtab *symtab, struct gen_record *rec)
{
  struct disp_bruggeman *brugg;
  disp_t *new_disp;
  struct toplevel_obj *res;
  struct value_lst * comp_lst;
  struct value_lst * frac_lst;
  str_ptr name;
  int j;

  if (rec->singleton)
    return NULL;

  name = get_named_field_as_string ("name", rec->cont.list);

  comp_lst = get_named_field_as_list ("components", rec->cont.list);
  if (comp_lst == NULL)
    return NULL;

  frac_lst = get_named_field_as_list ("fractions", rec->cont.list);
  if (frac_lst == NULL)
    return NULL;

  new_disp = disp_new_with_name (DISP_BRUGGEMAN, CSTR(name));
  brugg = & new_disp->disp.bruggeman;

  brugg->comp[0] = NULL;
  brugg->comp[1] = NULL;

  for (j = 0; /* empty */
       comp_lst && frac_lst && j < 2;
       comp_lst = comp_lst->next, frac_lst = frac_lst->next, j++)
    {
      struct value *frac_val = frac_lst->value;
      obj_t *disp;

      if (frac_val->type != VAL_TP_DOUBLE)
	break;

      disp = toplevel_eval_value (symtab, comp_lst->value);
      if (disp == NULL || disp->type != TL_TYPE_DISP)
	break;

      brugg->comp[j] = disp_copy (disp->cont.disp);
      brugg->frac[j] = frac_val->cont.real;
    }

  if (j < 2)
    goto brugg_err_exit;

  res = toplevel_obj_new (TL_TYPE_DISP, new_disp);

  return res;
 brugg_err_exit:
  disp_free (new_disp);
  return NULL;
}

obj_t *
lookup_model_interp (struct symtab *symtab, struct gen_record *rec)
{
  disp_t *new_dispersion;
  struct toplevel_obj *res;
  struct value_lst *plst, *dlst;
  struct lookup_comp *comp, *cptr;
  str_ptr name;
  double p0;
  int nbcomp;
  int err;

  if (rec->singleton)
    return NULL;

  name = get_named_field_as_string ("name", rec->cont.list);

  plst = get_named_field_as_list ("values", rec->cont.list);
  dlst = get_named_field_as_list ("dispersions", rec->cont.list);
  if (plst == NULL || dlst == NULL)
    return NULL;

  err = get_named_field_as_double ("predef", rec->cont.list, & p0);
  if (err)
    {
      struct value *v = plst->value;
      if (v->type != VAL_TP_DOUBLE)
	return NULL;
      p0 = v->cont.real;
    }

  nbcomp = valuelst_elements_nb (plst);
  comp = emalloc (nbcomp * sizeof(struct lookup_comp));
  
  for (cptr = comp; plst && dlst; cptr++, plst = plst->next, dlst = dlst->next)
    {
      struct value * pcomp = plst->value;
      obj_t * disp;

      disp = toplevel_eval_value (symtab, dlst->value);

      if (disp == NULL)
	break;

      if (disp->type != TL_TYPE_DISP || pcomp->type != VAL_TP_DOUBLE)
	break;

      cptr->p = pcomp->cont.real;
      cptr->disp = disp_copy (disp->cont.disp);

      toplevel_obj_free (disp);
    }

  if (cptr - comp < nbcomp)
    {
      free (comp);
      return NULL;
    }

  new_dispersion = disp_new_lookup (name ? CSTR(name) : "", nbcomp, comp, p0);
  res = toplevel_obj_new (TL_TYPE_DISP, new_dispersion);
  
  return res;
}

static obj_t *
read_disp_interp_raw (struct symtab *symtab, struct gen_record *rec,
		      disp_t *(*loader_func)(const char *))
{
  disp_t *new_dispersion;
  struct value *name;
  str_t fullname;

  if (! rec->singleton)
    return NULL;

  name = rec->cont.value;

  if (name->type != VAL_TP_STRING)
    return NULL;

  str_init (fullname, 64);
  if (str_is_abs_pathname (name->cont.string))
    {
      str_copy (fullname, name->cont.string);
    }
  else
    {
      str_copy (fullname, symtab->env->script_dir);
      str_append (fullname, name->cont.string, DIR_SEPARATOR);
    }

  new_dispersion = (*loader_func) (CSTR(fullname));

  str_free (fullname);

  if (new_dispersion == NULL)
    return NULL;

  return toplevel_obj_new (TL_TYPE_DISP, new_dispersion);
}

obj_t *
read_mat_file_interp (struct symtab *symtab, struct gen_record *rec)
{
  return read_disp_interp_raw (symtab, rec, load_mat_dispers);
}


obj_t *
read_nk_file_interp (struct symtab *symtab, struct gen_record *rec)
{
  return read_disp_interp_raw (symtab, rec, load_nk_table);
}
