/*
  $Id$
 */

#ifndef MULTI_FIT_ENGINE_H
#define MULTI_FIT_ENGINE_H

#include "defs.h"
#include "spectra.h"
#include "stack.h"
#include "fit-engine.h"
#include "cmpl.h"
#include "fit-engine-common.h"

#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h>

__BEGIN_DECLS

struct multi_fit_engine {
    enum system_kind system_kind;

    int samples_number;
    struct stack **stack_list;
    struct spectrum **spectra_list;

    const struct fit_parameters *common_parameters;
    const struct fit_parameters *private_parameters;

    struct extra_params extra;
    struct fit_config config;

    int initialized;

    gsl_multifit_function_fdf mffun;

    gsl_vector *results;
    gsl_vector *chisq;

    struct stack_cache cache;

    gsl_vector *jac_th;
    union {
        gsl_vector *refl;
        cmpl_vector *ell;
    } jac_n;
};

extern struct multi_fit_engine * \
multi_fit_engine_new(struct fit_config const *cfg,
                     int samples_number);

/* Bind the fit engine to the stack and fit parameters. */
extern void multi_fit_engine_bind(struct multi_fit_engine *fit, const stack_t *stack, const struct fit_parameters *cparameters, const struct fit_parameters *pparameters);

extern void multi_fit_engine_free(struct multi_fit_engine *f);
extern void multi_fit_engine_apply_parameters(struct multi_fit_engine *fit, const struct fit_parameters *fps, const double value[]);
extern int  multi_fit_engine_prepare(struct multi_fit_engine *f);

extern void multi_fit_engine_disable(struct multi_fit_engine *f);

extern int  multi_fit_engine_commit_parameters(struct multi_fit_engine *fit,
        const gsl_vector *x);

extern void multi_fit_engine_print_fit_results(struct multi_fit_engine *fit,
        str_t text);

__END_DECLS

#endif
