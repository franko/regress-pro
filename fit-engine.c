
/*
  $Id: fit-engine.c,v 1.4 2006/12/29 17:47:02 francesco Exp $
 */

#include <assert.h>
#include <string.h>
#include "fit-engine.h"
#include "refl-fit.h"
#include "refl-kernel.h"
#include "elliss-fit.h"
#include "elliss.h"
#include "symtab.h"
#include "error-messages.h"
#include "minsampling.h"


static int fit_engine_apply_param (struct fit_engine *fit,
				   const fit_param_t *fp, double val);

static void build_fit_engine_cache (struct fit_engine *f);

static void dispose_fit_engine_cache (struct fit_engine *f);


void
build_stack_cache (struct stack_cache *cache, stack_t *stack,
		   struct spectrum *spectr, int th_only_optimize)
{
  size_t nb_med = stack->nb;
  size_t j;

  cache->nb_med = nb_med;
  cache->ns  = emalloc (nb_med * sizeof(cmpl));

  cache->deriv_info = emalloc (nb_med * sizeof(struct deriv_info));

  for (j = 0; j < nb_med; j++)
    {
      struct deriv_info *di = cache->deriv_info + j;
      size_t tpnb = disp_get_number_of_params (stack->disp[j]);

      di->is_valid = 0;
      di->val = (tpnb == 0 ? NULL : cmpl_vector_alloc (tpnb));
    }

  cache->th_only = th_only_optimize;

  if (th_only_optimize)
    {
      int k, npt = spectra_points (spectr);
      cmpl *ns;

      cache->ns_full_spectr = emalloc (nb_med * npt * sizeof(cmpl));

      for (k = 0, ns = cache->ns_full_spectr; k < npt; ns += nb_med, k++)
	{
	  double lambda = get_lambda_by_index (spectr, k);
	  stack_get_ns_list (stack, ns, lambda);
	}
    }
  else
    {
      cache->ns_full_spectr = NULL;
    }

  cache->is_valid = 1;
}

void
dispose_stack_cache (struct stack_cache *cache)
{
  int j, nb_med = cache->nb_med;

  if (! cache->is_valid)
    return;

  free (cache->ns);

  for (j = 0; j < nb_med; j++)
    {
      struct deriv_info *di = & cache->deriv_info[j];
      if (di->val)
	cmpl_vector_free (di->val);
    }
  free (cache->deriv_info);

  if (cache->ns_full_spectr)
    free (cache->ns_full_spectr);

  cache->is_valid = 0;

  /* in principle the following instruction is not required:
     it could be removed once we are sure the program is bug free. */
  /* PS: in other words, this instruction will be never removed! :) */
  memset (cache, 0, sizeof(struct stack_cache));
}

void
build_fit_engine_cache (struct fit_engine *f)
{
  size_t nb = f->stack->nb;
  int nblyr = nb - 2;
  size_t dmultipl = (f->system_kind == SYSTEM_REFLECTOMETER ? 1 : 2);
  int RI_fixed = fit_parameters_are_RI_fixed (f->parameters);
  int th_only_optimize = RI_fixed && f->fixed_parameters;

  build_stack_cache (&f->cache, f->stack, f->spectr, th_only_optimize);

  f->jac_th = gsl_vector_alloc (dmultipl * nblyr);

  switch (f->system_kind)
    {
    case SYSTEM_REFLECTOMETER:
      f->jac_n.refl = gsl_vector_alloc (2 * nb);
      break;
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
      f->jac_n.ell = cmpl_vector_alloc (2 * nb);
    default:
      /* */;
    }
}

void
dispose_fit_engine_cache (struct fit_engine *f)
{
  gsl_vector_free (f->jac_th);

  switch (f->system_kind)
    {
    case SYSTEM_REFLECTOMETER:
      gsl_vector_free (f->jac_n.refl);
      f->jac_n.refl = NULL;
      break;
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
      cmpl_vector_free (f->jac_n.ell);
      f->jac_n.ell = NULL;
    default:
      /* */;
    }

  f->jac_th = NULL;

  dispose_stack_cache (& f->cache);
}

int
fit_engine_apply_param (struct fit_engine *fit, const fit_param_t *fp,
			double val)
{
  int res = 0;

  switch (fp->id)
    {
    case PID_FIRSTMUL:
      fit->extra->rmult = val;
      break;
    default:
      res = stack_apply_param (fit->stack, fp, val);
    }

  return res;
}

void
fit_engine_apply_parameters (struct fit_engine *fit,
			     const struct fit_parameters *fps, 
			     const gsl_vector *x)
{
  size_t j;

  for (j = 0; j < fps->number; j++)
    {
      const fit_param_t *fp = fps->values + j;
      double pval = gsl_vector_get (x, j);
      int status;
      status = fit_engine_apply_param (fit, fp, pval);
      /* No error should never occurs here because the fit parameters
	 are checked in advance. */
      assert (status == 0);
    }
}

void
fit_engine_commit_parameters (struct fit_engine *fit, const gsl_vector *x)
{
  struct fit_parameters const * fps = fit->parameters;
  fit_engine_apply_parameters (fit, fps, x);
}

int
fit_engine_prepare (struct fit_engine *fit, struct spectrum *s,
		    int fixed_parameters)
{
  struct fit_config *cfg = fit->config;

  if (s == NULL)
    return 1;

  fit->system_kind = s->config.system;

  fit->spectr = s;

  if (fit->config->spectr_range.active)
    spectr_cut_range (fit->spectr, 
		      fit->config->spectr_range.min,
		      fit->config->spectr_range.max);

  if (cfg->subsampling)
    {
      if (fit->system_kind == SYSTEM_ELLISS_AB ||
	  fit->system_kind == SYSTEM_ELLISS_PSIDEL)
	elliss_sample_minimize (fit->spectr, 0.05);
    }

  fit->fixed_parameters = fixed_parameters;

  build_fit_engine_cache (fit);

  switch (fit->system_kind)
    {
    case SYSTEM_REFLECTOMETER:
      fit->mffun.f      = & refl_fit_f;
      fit->mffun.df     = & refl_fit_df;
      fit->mffun.fdf    = & refl_fit_fdf;
      fit->mffun.n      = spectra_points (s);
      fit->mffun.p      = fit->parameters->number;
      fit->mffun.params = fit;
      break;
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
      fit->mffun.f      = & elliss_fit_f;
      fit->mffun.df     = & elliss_fit_df;
      fit->mffun.fdf    = & elliss_fit_fdf;
      fit->mffun.n      = 2 * spectra_points (s);
      fit->mffun.p      = fit->parameters->number;
      fit->mffun.params = fit;
      break;
    default:
      return 1;
    }

  if (! cfg->thresold_given)
    cfg->chisq_thresold = (fit->system_kind == SYSTEM_REFLECTOMETER ? 150 : 3000);

  fit->results = gsl_vector_alloc (fit->parameters->number);

  fit->initialized = 1;

#ifdef DEBUG_REGRESS
  if (fit->system_kind != SYSTEM_REFLECTOMETER)
    elliss_fit_test_deriv (fit);
#endif

  return 0;
}

void
fit_engine_disable (struct fit_engine *fit)
{
  dispose_fit_engine_cache (fit);
  fit->spectr = NULL;

  gsl_vector_free (fit->results);
  fit->results = NULL;

  fit->initialized = 0;
  fit->system_kind = SYSTEM_UNDEFINED;
}

void
fit_engine_restore_spectr (struct fit_engine *fit)
{
  data_view_reset (fit->spectr->table);
}

int
check_fit_parameters (struct stack *stack, struct fit_parameters *fps)
{
  size_t j, nb_med = (size_t) stack->nb;

  assert (stack->nb >= 2);

  for (j = 0; j < nb_med; j++)
    {
      if (disp_integrity_check (stack->disp[j]))
	{
	  notify_error_msg (INVALID_STRATEGY, "corrupted material card");
	  return 1;
	}
    }

  for (j = 0; j < fps->number; j++)
    {
      fit_param_t *fp = fps->values + j;
      switch (fp->id)
	{
	case PID_FIRSTMUL:
	  break;
	case PID_THICKNESS:
	  if (fp->layer_nb == 0 || fp->layer_nb >= nb_med - 1)
	    {
	      notify_error_msg (INVALID_STRATEGY, 
				"reference to thickness of layer %i",
				fp->layer_nb);
	      return 1;
	    }
	  break;
	case PID_LAYER_N:
	  if (fp->layer_nb >= nb_med)
	    {
	      notify_error_msg (INVALID_STRATEGY,
				"Reference to parameter of material number %i",
				fp->layer_nb);
	      return 1;
	    }

	  if (disp_check_fit_param (stack->disp[fp->layer_nb], fp))
	    {
	      str_t pname;
	      str_init (pname, 16);
	      get_param_name (fp, pname);
	      notify_error_msg (INVALID_STRATEGY,
				"Parameter %s makes no sense for layer %i",
				CSTR(pname), fp->layer_nb);
	      str_free (pname);
	      return 1;
	    }
	  break;
	default:
	  notify_error_msg (INVALID_STRATEGY, "ill-formed fit parameter");
	  return 1;
	}
    }
  
  return 0;
}

struct fit_parameters *
fit_engine_get_all_parameters (struct fit_engine *fit)
{
  struct stack *stack = fit->stack;
  int n_layers = stack->nb - 2;
  int n_params = 1 + n_layers;
  struct fit_parameters *fps;
  fit_param_t fp[1];
  int j;

  for (j = 1; j < n_layers + 2; j++)
    {
      disp_t *d = stack->disp[j];
      int np = disp_get_number_of_params (d);
      n_params += np;
    }

  fps = fit_parameters_new ();
  
  fp->id = PID_FIRSTMUL;
  fit_parameters_add (fps, fp);

  fp->id = PID_THICKNESS;
  for (j = 1; j < n_layers + 1; j++)
    {
      fp->layer_nb = j;
      fit_parameters_add (fps, fp);
    }

  fp->id = PID_LAYER_N;
  for (j = 1; j < n_layers + 2; j++)
    {
      disp_t *d = stack->disp[j];
      int k, np = disp_get_number_of_params (d);
      fp->layer_nb = j;
      fp->model_id = disp_get_model_id (d);
      for (k = 0; k < np; k++)
	{
	  fp->param_nb = k;
	  fit_parameters_add (fps, fp);
	}
    }

  return fps;
}

double
fit_engine_get_default_param_value (const struct fit_engine *fit, 
				    const fit_param_t *fp)
{
  if (fp->id == PID_FIRSTMUL)
    {
      return 1.0;
    }
  else if (fp->id == PID_THICKNESS)
    {
      const struct stack *st = fit->stack;
      int layer_nb = fp->layer_nb;
      assert(layer_nb > 0 && layer_nb < st->nb - 1);
      return st->thickness[layer_nb-1];
    }
  else if (fp->id == PID_LAYER_N)
    {
      const struct stack *st = fit->stack;
      int layer_nb = fp->layer_nb;
      const disp_t *d = st->disp[layer_nb];
      assert(layer_nb > 0 && layer_nb < st->nb - 1);
      return disp_get_param_value (d, fp);
    }

  assert(0);
  return 0.0;
}

struct spectrum *
generate_spectrum (struct fit_engine *fit)
{
  struct spectrum *synth;
  size_t nb_med = fit->stack->nb;
  struct data_table *table;
  struct { double const * ths; cmpl * ns; } actual;
  int j, npt = spectra_points (fit->spectr);
  const enum se_type se_type = GET_SE_TYPE(fit->system_kind);

  synth = emalloc (sizeof(struct spectrum));

  memcpy (&synth->config, &fit->spectr->config, sizeof(struct system_config));

  table = data_table_new (npt, 3);

  actual.ths = stack_get_ths_list (fit->stack);

  for (j = 0; j < npt; j++)
    {
      double lambda = get_lambda_by_index (fit->spectr, j);

      data_table_set (table, j, 0, lambda);

      actual.ns = fit->cache.ns;
      stack_get_ns_list (fit->stack, actual.ns, lambda);
      
      switch (fit->system_kind)
	{
	  double phi0;
	  double anlz;
	  double r_raw;
	  ell_ab_t ell;

	case SYSTEM_REFLECTOMETER:
	  r_raw = mult_layer_refl_ni (nb_med, actual.ns, actual.ths, lambda,
				      NULL, NULL);
	  data_table_set (table, j, 1, fit->extra->rmult * r_raw);
	  break;
	case SYSTEM_ELLISS_AB:
	case SYSTEM_ELLISS_PSIDEL:

	  phi0 = fit->spectr->config.aoi;
	  anlz = fit->spectr->config.analyzer;

	  mult_layer_se_jacob (se_type, nb_med, actual.ns, phi0,
			       actual.ths, lambda, anlz, ell, NULL, NULL);

	  data_table_set (table, j, 1, ell->alpha);
	  data_table_set (table, j, 2, ell->beta);
	  break;
	default:
	  /* */;
	}
    }

  data_view_init (synth->table, table);

  return synth;
}

struct fit_engine *
build_fit_engine (struct symtab *symtab, struct seeds **seeds)
{
  stack_t *stack;
  struct strategy *strategy;
  struct fit_engine *fit;
  
  stack    = retrieve_parsed_object (symtab, TL_TYPE_STACK,
				     symtab->directives->stack);

  strategy = retrieve_parsed_object (symtab, TL_TYPE_STRATEGY,
				     symtab->directives->strategy);

  if (stack == NULL || strategy == NULL)
    return NULL;

  if (check_fit_parameters (stack, strategy->parameters) != 0)
    return NULL;

  *seeds = strategy->seeds;

  fit = emalloc (sizeof(struct fit_engine));

  set_default_extra_param (fit->extra);

  memcpy (fit->config, symtab->config_table, sizeof(struct fit_config));

  /* fit is not the owner of the "parameters", we just keep a reference */
  fit->parameters = strategy->parameters;

  fit->stack = stack_copy (stack);

  fit->spectr = NULL;

  fit->system_kind = SYSTEM_UNDEFINED;
  fit->initialized = 0;
  fit->cache.is_valid = 0;

  fit->results = NULL;

  return fit;
}

int
fit_engine_set_parameters (struct fit_engine *fit,
			   struct fit_parameters *parameters)
{
  struct stack *stack = fit->stack;

  if (check_fit_parameters (stack, parameters) != 0)
    return 1;

  int old_pn = fit->results->size;
  int new_pn = parameters->number;

  fit->parameters = parameters;

  if (fit->initialized && old_pn != new_pn)
    {
      fit->mffun.p = new_pn;

      gsl_vector_free (fit->results);
      fit->results = gsl_vector_alloc (new_pn);
    }

  return 0;
}

void
fit_engine_free (struct fit_engine *fit)
{
  stack_free (fit->stack);
  free (fit);
}

void
set_default_extra_param (struct extra_params *extra)
{
  extra->rmult = 1.0;
}

void
fit_engine_print_fit_results (struct fit_engine *fit, str_t text, int tabular)
{
  str_t pname, value;
  size_t j;

  str_set_null (text);
  str_init (pname, 16);
  str_init (value, 16);

  for (j = 0; j < fit->parameters->number; j++)
    {
      fit_param_t *fp = fit->parameters->values + j;
      get_param_name (fp, pname);
      if (tabular)
	{
	  str_printf (value, "%.6g", gsl_vector_get (fit->results, j));
	  str_pad (value, 12, ' ');
	  str_append (text, value, 0);
	}
      else
	str_printf_add (text, "%9s : %.6g\n", CSTR(pname),
			gsl_vector_get (fit->results, j));
    }

  str_free (value);
  str_free (pname);
}
