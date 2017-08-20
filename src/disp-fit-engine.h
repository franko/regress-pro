#ifndef DISP_FIT_ENGINE_H
#define DISP_FIT_ENGINE_H

#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h>

#include "defs.h"
#include "dispers.h"
#include "fit-params.h"
#include "lmfit_result.h"

struct disp_fit_engine {

    disp_t *ref_disp;
    disp_t *model_disp;

    cmpl_vector *model_der;

    /* wavelength's sampling points */
    gsl_vector *wl;

    const struct fit_parameters *parameters;
};

struct disp_fit_config {
    int nb_max_iters;
    double chisq_norm_factor;
    double epsabs;
    double epsrel;
};

extern void disp_fit_config_init(struct disp_fit_config *fit);

extern struct disp_fit_engine * disp_fit_engine_new();
extern void disp_fit_engine_free(struct disp_fit_engine *fit);
extern void disp_fit_engine_set_parameters(struct disp_fit_engine *fit,
        const struct fit_parameters *fps);

extern int lmfit_disp(struct disp_fit_engine *fit, struct disp_fit_config *cfg,
                      gsl_vector *x, struct lmfit_result *result,
                      str_ptr analysis, str_ptr error_msg);

#endif
