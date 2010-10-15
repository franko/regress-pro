
/*
  $Id: interp.c,v 1.6 2006/07/12 22:48:54 francesco Exp $
 */

#include <string.h>
#include "interp.h"
#include "model-interp.h"
#include "error-messages.h"
#include "multi-fit-interp.h"
#include "dispers-library.h"

#undef INTERP_DEBUG

static obj_t * parse_record (struct symtab *symtab, struct gen_record * rec);
static obj_t * stack_interp (struct symtab *symtab, struct gen_record *rec);
static obj_t * fit_interp   (struct symtab *symtab, struct gen_record *rec);

static obj_t * library_disp_interp (struct symtab *symtab,
				    struct gen_record *rec);

static const struct toplevel_parser * find_record_parser (str_ptr recid);

#define PARSER_TB_SIZE 11
static const size_t nb_parsers = PARSER_TB_SIZE;
static const struct toplevel_parser parser_table[PARSER_TB_SIZE] = {
  {"CauchyDispersion", cauchymodel_interp},
  {"Stack",            stack_interp},
  {"Strategy",         fit_interp},
  {"HODispersion",     ho_model_interp},
  {"Bruggeman",        bruggeman_model_interp},
  {"ReadNKFile",       read_nk_file_interp},
  {"ReadMATFile",      read_mat_file_interp},
  {"LookupDispersion", lookup_model_interp},
  {"Sample",           sample_interp},
  {"MultiFit",         multi_fit_interp},
  {"Library",          library_disp_interp},
};

int
valuelst_elements_nb (struct value_lst *lst)
{
  int count = 0;
  while (lst)
    {
      count ++;
      lst = lst->next;
    }
  return count;
}

obj_t *
stack_interp (struct symtab *symtab, struct gen_record *rec)
{
  struct value_lst * vlst;
  stack_t *stack;
  struct pair_lst *args;
  int nbmed, j;

  if (rec->singleton)
    return NULL;

  args = rec->cont.list;

  stack = emalloc (sizeof(stack_t));

  vlst = get_named_field_as_list ("mediums", args);

  nbmed = valuelst_elements_nb (vlst);
  stack_init_raw (stack, nbmed);

  for (j = 0; vlst && j < nbmed; vlst = vlst->next, j++, stack->nb++)
    {
      struct value *v = vlst->value;
      struct toplevel_obj *evres;
      evres = toplevel_eval_value (symtab, v);

      if (evres == NULL)
	{
	  notify_error_msg (SCRIPT_ERROR, "invalid dispertion specified");
	  goto clean_ret;
	}

      if (evres->type != TL_TYPE_DISP)
	{
	  notify_error_msg (SCRIPT_ERROR, "a dispertion should be specified.");
	  goto clean_ret;
	}

      stack->disp[j] = disp_copy (evres->cont.disp);

      toplevel_obj_free (evres);
    }

  if (stack->nb <= 2 || j < nbmed)
    {
      const char *xmsg = (stack->nb <= 2 ?
			  "at least one layer should be present." :
			  "invalid dispersion name");
      notify_error_msg (SCRIPT_ERROR, xmsg);
      goto clean_ret;
    }

  vlst = get_named_field_as_list ("thicknesses", rec->cont.list);
  for (j = 0; j < stack->nb - 2 && vlst; vlst = vlst->next, j++)
    {
      struct value *v = vlst->value;

      if (v->type != VAL_TP_DOUBLE)
	{
	  notify_error_msg (SCRIPT_ERROR, "expecting floating point value.");
	  goto clean_ret;
	}

      stack->thickness[j] = v->cont.real;
    }

  if (j < stack->nb - 2 || vlst)
    {
      notify_error_msg (SCRIPT_ERROR, "wrong number of initializers");
      goto clean_ret;
    }

  return toplevel_obj_new (TL_TYPE_STACK, stack);

 clean_ret:
  stack_free (stack);
  return NULL;
}

struct toplevel_obj *
fit_interp (struct symtab *symtab, struct gen_record *rec)
{
  struct value_lst * vlst, *vlst_seed;
  struct strategy *strategy;

  if (rec->singleton)
    return NULL;

  strategy = strategy_new ();

  vlst = get_named_field_as_list ("params", rec->cont.list);
  vlst_seed = get_named_field_as_list ("seeds", rec->cont.list);
  for (; vlst && vlst_seed; vlst = vlst->next, vlst_seed = vlst_seed->next)
    {
      struct value *v = vlst->value;
      struct value *seed_value = vlst_seed->value;
      fit_param_t fp[1];
      int decode_error;

      if (v->type != VAL_TP_STRING)
	{
	  notify_error_msg (SCRIPT_ERROR,
			    "expecting string as parameter name.");
	  goto clean_fit;
	}

      if (seed_value->type == VAL_TP_DOUBLE)
	{
	  seed_list_add_simple (strategy->seeds, seed_value->cont.real);
	}
      else if (seed_value->type == VAL_TP_STRING)
	{
	  seed_t seed[1];
	  int status;

	  status = parse_fit_string (CSTR(seed_value->cont.string), seed);

	  if (status)
	    {
	      notify_error_msg (SCRIPT_ERROR, "wrong range specified.");
	      goto clean_fit;
	    }

	  seed_list_add (strategy->seeds, seed);
	}
      else
	{
	  notify_error_msg (SCRIPT_ERROR,
			    "a string or floating point value should be "
			    "provided\nas seed/range specification.");
	  goto clean_fit;
	}
	  
      decode_error = decode_fit_param (fp, v->cont.string);
      if (decode_error)
	{
	  notify_error_msg (SCRIPT_ERROR, "wrong fit parameter \"%s\"",
			    CSTR(v->cont.string));
	  goto clean_fit;
	}

      fit_parameters_add (strategy->parameters, fp);
    }

  if (vlst || vlst_seed)
    {
      notify_error_msg (SCRIPT_ERROR, "wrong number of initializers");
      goto clean_fit;
    }

  return toplevel_obj_new (TL_TYPE_STRATEGY, strategy);

 clean_fit:
  strategy_free (strategy);
  return NULL;
}

struct value *
get_named_field (const char *name, struct pair_lst *lst)
{
  while (lst)
    {
      struct pair *pair = lst->field;
      if (strcmp (CSTR(pair->key), name) == 0)
	return pair->value;
      lst = lst->next;
    }
  
  return NULL;
}

str_ptr
get_named_field_as_string (const char *name, struct pair_lst *lst)
{
  struct value *v = get_named_field (name, lst);
  if (v)
    if (v->type == VAL_TP_STRING)
      return v->cont.string;
  return NULL;
}

struct value_lst *
get_named_field_as_list (const char *name, struct pair_lst *lst)
{
  struct value *v = get_named_field (name, lst);
  if (v)
    if (v->type == VAL_TP_LIST)
      return v->cont.vallist;
  return NULL;
}

int
get_named_field_as_double (const char *name, struct pair_lst *lst, double *x)
{
  struct value *v = get_named_field (name, lst);
  if (v)
    if (v->type == VAL_TP_DOUBLE)
      {
       *x = v->cont.real;
       return 0;
      }

  return 1;
}

const struct toplevel_parser *
find_record_parser (str_ptr recid)
{
  size_t j;
  for (j = 0; j < nb_parsers; j++)
    {
      const struct toplevel_parser *p = parser_table + j;
      if (strcmp (p->id, CSTR(recid)) == 0)
	return p;
    }
  return NULL;
}

struct toplevel_obj *
parse_record (struct symtab *symtab, struct gen_record * rec)
{
  struct toplevel_obj *result;
  const struct toplevel_parser *parser = find_record_parser (rec->id);
  if (parser == NULL)
    {
      notify_error_msg (SCRIPT_ERROR, 
			"record name \"%s\" not recognized", CSTR(rec->id));
      return NULL;
    }
#ifdef INTERP_DEBUG
  fprintf (stderr, "Using record parser: %s\n", parser->id);
#endif
  result = (*parser->func)(symtab, rec);
  return result;
}

int
parse_assignment (struct symtab *symtab, struct pair * p)
{
  str_ptr name = p->key;
  struct gen_record * rec;
  struct toplevel_obj * result;

  if (p->value->type != VAL_TP_RECORD)
    {
      notify_error_msg (SCRIPT_ERROR, "right-hand member is not a record");
      return 1;
    }

  rec = p->value->cont.record;

#ifdef INTERP_DEBUG
  fprintf (stderr, "Parsing assignment: %s\n", CSTR(name));
#endif
  
  result = parse_record (symtab, rec);

  if (result == NULL)
    return 1;

  symbol_table_add (symtab, name, result);

  return 0;
}

obj_t *
toplevel_eval_value (struct symtab *symtab, struct value *v)
{
  obj_t *res = NULL;

  switch (v->type)
    {
    case VAL_TP_VARREF:
      res = symbol_table_find (symtab, v->cont.string);
      if (res)
	res = toplevel_obj_copy (res);
      break;
    case VAL_TP_RECORD:
      res = parse_record (symtab, v->cont.record);
      break;
    default:
      /* */ ;
    }

  return res;
}

obj_t *
library_disp_interp (struct symtab *symtab, struct gen_record *rec)
{
  disp_t *new_dispersion;
  struct value *name;

  if (! rec->singleton)
    return NULL;

  name = rec->cont.value;

  if (name->type != VAL_TP_STRING)
    return NULL;

  new_dispersion = dispers_library_search (CSTR(name->cont.string));
  if (new_dispersion == NULL)
    {
      notify_error_msg (SCRIPT_ERROR, 
			"cannot find \"%s\" in dispersion library",
			CSTR(name->cont.string));
      return NULL;
    }

  return toplevel_obj_new (TL_TYPE_DISP, new_dispersion);
}
