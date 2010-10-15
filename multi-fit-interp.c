
/*
  $Id$
 */

#include <assert.h>
#include <string.h>

#include "sample-info.h"
#include "multi-fit-interp.h"
#include "spectra.h"
#include "error-messages.h"

static int get_fit_param_from_value (fit_param_t *fp, struct value *v);


obj_t *
sample_interp (struct symtab *symtab, struct gen_record *rec)
{
  struct value_lst *constraints, *individual;
  str_ptr name;
  struct sample_info *sample_info;
  int constr_nb, indiv_nb;
  size_t j;

  if (rec->singleton)
    return NULL;

  name = get_named_field_as_string ("spectrum", rec->cont.list);
  if (name == NULL)
    {
      notify_error_msg (SCRIPT_ERROR,
			"missing declaration of spectrum name");
      return NULL;
    }

  /* constraints can be NULL if we have an empty list */
  constraints = get_named_field_as_list ("constraints", rec->cont.list);

  /* individual can be NULL if we have an empty list */
  individual = get_named_field_as_list ("individual", rec->cont.list);

  constr_nb = valuelst_elements_nb (constraints);
  indiv_nb  = valuelst_elements_nb (individual);

  sample_info = sample_info_new (constr_nb, indiv_nb);

  str_copy (sample_info->spectrum_name, name);

  for (j = 0; constraints; constraints = constraints->next, j++)
    {
      struct value *v = constraints->value;

      if (v->type != VAL_TP_DOUBLE)
	{
	  notify_error_msg (SCRIPT_ERROR,
			    "errror in constraints definition");
	  goto clean_sample;
	}

      sample_info->constraints[j] = v->cont.real;
    }

  for (j = 0; individual; individual = individual->next, j++)
    {
      struct value *v = individual->value;

      if (v->type != VAL_TP_DOUBLE)
	{
	  notify_error_msg (SCRIPT_ERROR,
			    "error in individual parameters definition");
	  goto clean_sample;
	}

      sample_info->individual[j] = v->cont.real;
    }

  return toplevel_obj_new (TL_TYPE_SAMPLE, sample_info);

 clean_sample:
  sample_info_free (sample_info);
  return NULL;
}

int
get_fit_param_from_value (fit_param_t *fp, struct value *v)
{
  int decode_error;
  
  if (v->type != VAL_TP_STRING)
    {
      notify_error_msg (SCRIPT_ERROR,
			"expecting string as parameter specification");
      return 1;
    }

  decode_error = decode_fit_param (fp, v->cont.string);
  if (decode_error)
    {
      notify_error_msg (SCRIPT_ERROR, "wrong fit parameter \"%s\"",
			CSTR(v->cont.string));
      return 1;
    }

  return 0;
}

obj_t *
multi_fit_interp (struct symtab *symtab, struct gen_record *rec)
{
  struct value_lst *con_lst, *ind_lst, *sam_lst;
  struct multi_fit_info *info;
  int sample, samples_number;
  int constr_nb, indiv_nb;

  con_lst = get_named_field_as_list ("constraints", rec->cont.list);
  ind_lst = get_named_field_as_list ("individual",  rec->cont.list);
  sam_lst = get_named_field_as_list ("samples",     rec->cont.list);

  if (sam_lst == NULL)
    {
      notify_error_msg (SCRIPT_ERROR,
			"error in definition of samples");
      return NULL;
    }

  samples_number = valuelst_elements_nb (sam_lst);
  constr_nb      = valuelst_elements_nb (con_lst); 
  indiv_nb       = valuelst_elements_nb (ind_lst); 

  info = multi_fit_info_new (samples_number);

  for (; con_lst; con_lst = con_lst->next)
    {
      fit_param_t fp[1];

      if (get_fit_param_from_value (fp, con_lst->value) != 0)
	goto clean_multi;

      fit_parameters_add (info->constraints.parameters, fp);
    }

  for (; ind_lst; ind_lst = ind_lst->next)
    {
      fit_param_t fp[1];

      if (get_fit_param_from_value (fp, ind_lst->value) != 0)
	goto clean_multi;

      fit_parameters_add (info->individual.parameters, fp);
    }

  for (sample = 0; sam_lst; sam_lst = sam_lst->next, sample++)
    {
      struct toplevel_obj *sample_obj;
      struct sample_info *sample_info;
      struct spectrum *spectrum;
      int k;

      sample_obj = toplevel_eval_value (symtab, sam_lst->value);

      if (sample_obj == NULL)
	{
	  notify_error_msg (SCRIPT_ERROR,
			    "invalid sample specified");
	  goto clean_multi;
	}

      if (sample_obj->type != TL_TYPE_SAMPLE)
	{
	  notify_error_msg (SCRIPT_ERROR, "invalid sample specified");
	  toplevel_obj_free (sample_obj);
	  goto clean_multi;
	}

      sample_info = sample_obj->cont.sample_info;

      spectrum = load_gener_spectrum (CSTR(sample_info->spectrum_name));
      if (spectrum == NULL)
	{
	  notify_error_msg (LOADING_FILE_ERROR,
			    "cannot load spectra file %s",
			    CSTR(sample_info->spectrum_name));
	  toplevel_obj_free (sample_obj);
	  goto clean_multi;
	}

      info->spectra_list[sample] = spectrum;

      for (k = 0; k < sample_info->constr_nb; k++)
	{
	  seed_list_add_simple (info->constraints.seeds,
				sample_info->constraints[k]);
	}

      for (k = 0; k < sample_info->indiv_nb; k++)
	{
	  seed_list_add_simple (info->individual.seeds,
				sample_info->individual[k]);
	}

      toplevel_obj_free (sample_obj);
    }

  return toplevel_obj_new (TL_TYPE_MULTI_FIT, info);

 clean_multi:

  multi_fit_info_free (info);

  return NULL;
}
