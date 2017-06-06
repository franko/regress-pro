#ifndef MULTI_FIT_ENGINE_H
#define MULTI_FIT_ENGINE_H

#include "defs.h"
#include "spectra.h"
#include "stack.h"
#include "fit-engine.h"
#include "cmpl.h"
#include "fit-engine-common.h"
#include "acquisition.h"

#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h>

__BEGIN_DECLS


struct multi_fit_engine {
    int samples_number;
    struct stack **stack_list;
    struct spectrum **spectra_list;

    const struct fit_parameters *common_parameters;
    const struct fit_parameters *private_parameters;

    struct acquisition_parameters *acquisitions;
    struct fit_config config;

    int initialized;

    gsl_multifit_function_fdf mffun;

    gsl_vector *results;
    gsl_vector *chisq;

    struct stack_cache cache;
};

extern struct multi_fit_engine * \
multi_fit_engine_new(struct fit_config const *cfg,
                     int samples_number);

/* Bind the fit engine to the stack and fit parameters. */
extern void multi_fit_engine_bind(struct multi_fit_engine *fit, const stack_t *stack, const struct fit_parameters *cparameters, const struct fit_parameters *pparameters);

extern void multi_fit_engine_free(struct multi_fit_engine *f);
extern void multi_fit_engine_apply_parameters(struct multi_fit_engine *fit, int sample_nb, const struct fit_parameters *fps, const double value[]);
extern int  multi_fit_engine_prepare(struct multi_fit_engine *f);

extern void multi_fit_engine_disable(struct multi_fit_engine *f);

extern int  multi_fit_engine_commit_parameters(struct multi_fit_engine *fit,
        const gsl_vector *x);

extern void multi_fit_engine_print_fit_results(struct multi_fit_engine *fit,
        str_t text);

extern double multi_fit_engine_get_parameter_value(const struct multi_fit_engine *fit, const fit_param_t *fp);
extern double multi_fit_engine_get_seed_value(const struct multi_fit_engine *fit, const fit_param_t *fp, const seed_t *s);
extern void multi_fit_engine_compute_chisq(struct multi_fit_engine *fit, const gsl_vector *f);

#ifdef DEBUG
extern void multi_fit_engine_check_deriv(struct multi_fit_engine *fit, struct seeds *seeds_common, struct seeds *seeds_priv);
#endif

__END_DECLS

#endif