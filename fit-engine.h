/*
  $Id: fit-engine.h,v 1.4 2006/12/29 17:47:02 francesco Exp $
 */

#ifndef FIT_ENGINE_H
#define FIT_ENGINE_H

#include "defs.h"
#include "elliss.h"
#include "spectra.h"
#include "stack.h"
#include "fit-params.h"
#include "fit-engine-common.h"

#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h>

__BEGIN_DECLS

struct extra_params {
  /* Reflectometry parameters */
  double rmult;
};

struct fit_engine {

  enum system_kind system_kind;

  struct stack *stack;
  struct fit_parameters *parameters;

  struct spectrum *spectr;

  struct extra_params extra[1];
  struct fit_config config[1];

  int initialized;

  gsl_multifit_function_fdf mffun;

  gsl_vector *results;

  struct stack_cache cache;

  gsl_vector *jac_th;
  union {
    gsl_vector *refl;
    cmpl_vector *ell;
  } jac_n;
};

#define GET_SE_TYPE(sk) (sk == SYSTEM_ELLISS_AB ? SE_ALPHA_BETA : SE_PSI_DEL)

struct seeds;
struct symtab;

extern void fit_engine_free                 (struct fit_engine *fit);

extern int  fit_engine_prepare              (struct fit_engine *f,
					     struct spectrum *s);

extern void fit_engine_disable              (struct fit_engine *f);

extern int  check_fit_parameters            (struct stack *stack,
					     struct fit_parameters *fps);

extern int  fit_engine_commit_parameters    (struct fit_engine *fit,
					     const gsl_vector *x);

extern void fit_engine_restore_spectr       (struct fit_engine *fit);


extern void build_stack_cache               (struct stack_cache *cache,
					     stack_t *stack,
					     struct spectrum *spectr,
					     int RIs_are_fixed);

extern void dispose_stack_cache             (struct stack_cache *cache);

extern struct fit_engine * build_fit_engine (struct symtab *symtab,
					     struct seeds **seeds);

extern void set_default_extra_param         (struct extra_params *extra);

extern struct spectrum * generate_spectrum  (struct fit_engine *fit);

extern void fit_engine_print_fit_results    (struct fit_engine *fit,
					     str_t text, int tabular);

extern struct fit_parameters *
fit_engine_get_all_parameters (struct fit_engine *fit);

extern double
fit_engine_get_default_param_value (const struct fit_engine *fit, 
				    const fit_param_t *fp);

extern int
fit_engine_set_parameters (struct fit_engine *fit,
			   struct fit_parameters *parameters);

__END_DECLS

#endif
