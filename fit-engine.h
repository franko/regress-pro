
/* fit-engine.h
 * 
 * Copyright (C) 2005-2011 Francesco Abbate
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

struct fit_run {
  enum system_kind system_kind;

  struct spectrum *spectr;

  gsl_multifit_function_fdf mffun;

  gsl_vector *results;

  struct stack_cache cache;

  gsl_vector *jac_th;
  union {
    gsl_vector *refl;
    cmpl_vector *ell;
  } jac_n;
};

struct fit_engine {
  struct extra_params extra[1];
  struct fit_config config[1];

  struct stack *stack;
  struct fit_parameters *parameters;

  struct fit_run run[1];
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

extern void fit_engine_commit_parameters    (struct fit_engine *fit,
					     const gsl_vector *x);

extern int fit_engine_apply_param           (struct fit_engine *fit,
					     const fit_param_t *fp, double val);

extern void fit_engine_apply_parameters     (struct fit_engine *fit,
					     const struct fit_parameters *fps, 
					     const gsl_vector *x);

extern void build_stack_cache               (struct stack_cache *cache,
					     stack_t *stack,
					     struct spectrum *spectr,
					     int th_only_optimize);

extern void dispose_stack_cache             (struct stack_cache *cache);

extern struct fit_engine * build_fit_engine (struct symtab *symtab,
					     struct seeds **seeds);

extern void set_default_extra_param         (struct extra_params *extra);

extern void fit_engine_generate_spectrum (struct fit_engine *fit,
					  struct spectrum *ref,
					  struct spectrum *synth);

extern void fit_engine_print_fit_results    (struct fit_engine *fit,
					     str_t text, int tabular);

extern struct fit_parameters *
fit_engine_get_all_parameters (struct fit_engine *fit);

extern double
fit_engine_get_parameter_value (const struct fit_engine *fit, 
				const fit_param_t *fp);

__END_DECLS

#endif
