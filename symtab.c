#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"
#include "str-util.h"
#include "symtab.h"
#include "error-messages.h"
#include "fitlexer.h"

extern int yyparse (struct symtab *symtab);

static struct assign * symbol_table_cell_find (struct symtab *symtab,
					       str_ptr name);
static int             set_config_value_raw   (struct symtab *symtab,
					       lex_string_t *id, val_t *v);
static void            config_set_default     (struct fit_config *cfg);


static char const * const tl_type_name[] = {
  "dispersion", "stack", "strategy", "sample", "multiple fit informations"
};

void
symbol_table_init (struct symtab symtab[])
{
  symtab->assign_start = NULL;

  str_init (symtab->directives->stack, 16);
  str_init (symtab->directives->strategy, 16);
  str_init (symtab->directives->multi_fit, 16);

  str_init (symtab->env->script_dir, 64);
  str_getcwd (symtab->env->script_dir);
}

void
symbol_table_free (struct symtab symtab[])
{
  str_free (symtab->directives->stack);
  str_free (symtab->directives->strategy);
  str_free (symtab->directives->multi_fit);
  str_free (symtab->env->script_dir);
}

struct assign *
symbol_table_next (struct symtab *symtab, struct assign *iter)
{
  struct assign *next = (iter ? iter->next : symtab->assign_start);
  return next;
}

obj_t *
toplevel_obj_new (enum toplevel_type kind, void *content)
{
  obj_t *res = emalloc (sizeof(obj_t));

  assert (kind < TL_TYPE_EXCEED_VALUE);

  res->type = kind;
  /* we can use the generic pointer because the cont field is a union
     of pointers fields */
  res->cont.generic = content;

  return res;
}

obj_t *
toplevel_obj_copy (obj_t *obj)
{
  obj_t *res = emalloc (sizeof(obj_t));
  memcpy (res, obj, sizeof(obj_t));
  switch (obj->type)
    {
    case TL_TYPE_DISP:
      obj->cont.disp = disp_copy (res->cont.disp);
      break;
    case TL_TYPE_STACK:
      obj->cont.stack = stack_copy (res->cont.stack);
      break;
    case TL_TYPE_SAMPLE:
      obj->cont.sample_info = sample_info_copy (res->cont.sample_info);
      break;
    default:
      /* not needed */
      assert (0);
    }

  return res;
}

void
toplevel_obj_free (obj_t *obj)
{
  switch (obj->type)
    {
    case TL_TYPE_DISP:
      disp_free (obj->cont.disp);
      break;
    case TL_TYPE_STACK:
      stack_free (obj->cont.stack);
      break;
    case TL_TYPE_STRATEGY:
      strategy_free (obj->cont.strategy);
      break;
    case TL_TYPE_SAMPLE:
      str_free (obj->cont.sample_info->spectrum_name);
      free (obj->cont.sample_info->constraints);
      free (obj->cont.sample_info->individual);
      free (obj->cont.sample_info);
      break;
    case TL_TYPE_MULTI_FIT:
      multi_fit_info_free (obj->cont.multi_fit);
    default:
      /* */ ;
    }
  free (obj);
}

struct assign *
symbol_table_cell_find (struct symtab *symtab, str_ptr name)
{
  struct assign *cell = symtab->assign_start;
  while (cell)
    {
      if (strcmp (CSTR(cell->name), CSTR(name)) == 0)
	break;
      cell = cell->next;
    }
  return cell;
}

void
symbol_table_add (struct symtab *symtab, str_ptr name, obj_t *val)
{
  struct assign *cell = symbol_table_cell_find (symtab, name);
  if (cell)
    {
      toplevel_obj_free (cell->value);
      cell->value = val;
    }
  else
    {
      struct assign *neo;

      cell = symtab->assign_start;
      while (cell)
	{
	  if (cell->next == NULL)
	    break;
	  cell = cell->next;
	}

      neo = emalloc (sizeof(struct assign));
      str_init_from_str (neo->name, name);
      neo->value = val;
      neo->next = NULL;

      if (cell)
	cell->next = neo;
      else
	symtab->assign_start = neo;
    }
}

obj_t *
symbol_table_find (struct symtab *symtab, str_ptr name)
{
  struct assign *cell = symbol_table_cell_find (symtab, name);
  if (cell)
    return cell->value;
  return NULL;
}

void
symbol_table_clean (struct symtab *symtab)
{
  struct assign *p = symtab->assign_start;

  while (p)
    {
      struct assign *next = p->next;
      str_free (p->name);
      toplevel_obj_free (p->value);
      free (p);
      p = next;
    }

  symtab->assign_start = NULL;

  str_set_null (symtab->directives->stack);
  str_set_null (symtab->directives->strategy);
  str_set_null (symtab->directives->multi_fit);
}

#if 0
obj_t *
symbol_table_unlink (struct symtab *symtab, 
		     enum toplevel_type wanttp, str_ptr wantnm)
{
  obj_t *result;
  struct assign *p, *top = NULL;

  for (p = symtab->assign_start; p; top = p, p = p->next)
    {
      if (p->value->type == wanttp)
	{
	  if (strcmp (CSTR(p->name), CSTR(wantnm)) == 0)
	    break;
	}
    }

  if (p == NULL)
    return NULL;

  if (top == NULL)
    symtab->assign_start = p->next;
  else
    top->next = p->next;

  result = p->value;

  str_free (p->name);
  free (p);

  return result;
}
#endif

int
setup_use_directive (struct symtab *symtab, 
		     lex_string_t *id, lex_string_t *val)
{
  int status = 0;
  str_ptr neo = get_lex_string (val);

  if (neo == NULL)
    return 1;

  if (LEX_STRING_EQUAL(id, "stack"))
    str_copy (symtab->directives->stack, neo);
  else if (LEX_STRING_EQUAL(id, "strategy"))
    str_copy (symtab->directives->strategy, neo);
  else if (LEX_STRING_EQUAL(id, "multi_fit"))
    str_copy (symtab->directives->multi_fit, neo);
  else
    status = 1;

  STR_PTR_FREE(neo);

  return status;
}

void *
retrieve_parsed_object (struct symtab *symtab, enum toplevel_type tp,
			str_ptr name)
{
  obj_t *obj;

  obj = symbol_table_find (symtab, name);

  if (obj == NULL)
    {
      notify_error_msg (SCRIPT_ERROR, "default %s not defined",
			tl_type_name[tp]);
      return NULL;
    }

  if (obj->type != tp)
    {
      notify_error_msg (SCRIPT_ERROR, "default %s is invalid",
			tl_type_name[tp]);
      return NULL;
    }

  return obj->cont.generic;
}

int
set_config_value_raw (struct symtab *symtab, lex_string_t *id, val_t *v)
{
  struct fit_config *cfg = symtab->config_table;

  if (LEX_STRING_EQUAL(id, "range"))
    {
      if (v->type == VAL_TP_STRING)
	{
	  const char *s = CSTR(v->cont.string);
	  char *tail;
	  double inf, sup;
	  inf = strtod (s, &tail);
	  if (tail == s || *tail != '-')
	    goto range_exit_err;
	  s = tail + 1;
	  sup = strtod (s, &tail);
	  if (tail == s || *tail != 0)
	    goto range_exit_err;
	  cfg->spectr_range.active = 1;
	  cfg->spectr_range.min = inf;
	  cfg->spectr_range.max = sup;
	  return 0;
	}
    range_exit_err:
      notify_error_msg (SCRIPT_ERROR, "Invalid range specification.");
      return 1;
    }
  else if (LEX_STRING_EQUAL(id, "chisq_thresold"))
    {
      if (v->type == VAL_TP_DOUBLE)
	{
	  cfg->chisq_thresold = v->cont.real;
	  cfg->thresold_given = 1;
	  return 0;
	}

    }
  else if (LEX_STRING_EQUAL(id, "max_iterations"))
    {
      if (v->type == VAL_TP_DOUBLE)
	{
	  cfg->nb_max_iters = (int) v->cont.real;
	  return 0;
	}
    }
  else if (LEX_STRING_EQUAL(id, "subsampling"))
    {
      if (v->type == VAL_TP_STRING)
	{
	  if (strcmp (CSTR(v->cont.string), "on") == 0)
	    {
	      cfg->subsampling = 1;
	      return 0;
	    }
	  else if (strcmp (CSTR(v->cont.string), "off") == 0)
	    {
	      cfg->subsampling = 0;
	      return 0;
	    }
	  notify_error_msg (SCRIPT_ERROR, "subsampling should be either "
			    "\"on\" or \"off\" (with quotes).");
	  return 1;
	}
    }
  else if (LEX_STRING_EQUAL(id, "directory"))
    {
      if (v->type == VAL_TP_STRING)
	{
	  int status = chdir (CSTR(v->cont.string));
	  if (status != 0)
	    {
	      notify_error_msg (SCRIPT_ERROR, "%s.", strerror (errno));
	      return 1;
	    }
	  return 0;
	}
    }
  else if (LEX_STRING_EQUAL(id, "wavelength_integ"))
    {
      if (v->type == VAL_TP_DOUBLE)
	{
	  cfg->wavelength_integ = (int) v->cont.real;
	  return 0;
	}
    }

  notify_error_msg (SCRIPT_ERROR, "Invalid value.");
  return 1;
}

int
set_config_value (struct symtab *symtab, lex_string_t *id, val_t *v)
{
  int status = set_config_value_raw (symtab, id, v);
  value_free (v);
  return status;
}


void
config_set_default (struct fit_config *cfg)
{
  cfg-> wavelength_integ = 0;
  cfg->thresold_given = 0;
  cfg->nb_max_iters = 30;
  cfg->subsampling = 1;
  cfg->spectr_range.active = 0;
  cfg->epsabs = 1.0E-7;
  cfg->epsrel = 1.0E-7;
}

int
parse_strategy (struct symtab *symtab, const char *text)
{
  str_t workdir;
  int status;

  str_init (workdir, 64);
  str_getcwd (workdir);

  clean_error_msgs ();

  lexer_init (text);

  symbol_table_clean (symtab);

  config_set_default (symtab->config_table);

  status = yyparse (symtab);

  chdir (CSTR(workdir));
  str_free (workdir);

  if (status != 0)
    symbol_table_clean (symtab);

  return status;
}
