#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <assert.h>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

#include "refl-fit.h"
#include "refl-kernel.h"
#include "fit-engine.h"
#include "refl-get-jacobian.h"

double
get_parameter_jacob_r(fit_param_t const *fp, stack_t const *stack,
                      struct deriv_info *ideriv, double lambda,
                      double jacob_th[], cmpl jacob_n[], double jacob_acq[1])
{
    double result;
    int lyr = fp->layer_nb;

    switch(fp->id) {
        double dnr, dni;

    case PID_THICKNESS:
        result = jacob_th[lyr - 1];
        break;
    case PID_LAYER_N:
        get_model_param_deriv(stack->disp[lyr], &ideriv[lyr],
                              fp, lambda, &dnr, &dni);

        const cmpl drdn = jacob_n[lyr];
        result = creal(drdn) * dnr - cimag(drdn) * dni;
        break;
    case PID_BANDWIDTH:
        result = jacob_acq[SR_BANDWIDTH];
        break;
    case PID_FIRSTMUL:
        result = jacob_acq[SR_RMULT];
        break;
    default:
        result = 0.0;
    }

    return result;
}

int
refl_fit_fdf(const gsl_vector *x, void *params,
             gsl_vector *f, gsl_matrix * jacob)
{
    struct fit_engine *fit = params;
    struct spectrum *s = fit->run->spectr;
    size_t nb_med = fit->stack->nb;
    double jacob_th_data[nb_med - 2];
    cmpl jacob_n_data[nb_med];
    double jacob_acq_data[SR_ACQ_PARAMETERS_NB];

    /* STEP 1 : We apply the actual values of the fit parameters
                to the stack. */

    fit_engine_commit_parameters(fit, x);

    /* STEP 2 : From the stack we retrive the thicknesses and RIs
                informations. */

    const double *ths = stack_get_ths_list(fit->stack);

    double *jacob_th  = (jacob ? jacob_th_data : NULL);
    cmpl *  jacob_n   = (jacob && !fit->run->cache.th_only ? jacob_n_data  : NULL);
    double *jacob_acq = (jacob && fit->run->cache.require_acquisition_jacob ? jacob_acq_data : NULL);

    for(int j = 0; j < spectra_points(s); j++) {
        float const * spectr_data = spectra_get_values(s, j);
        const double lambda = spectr_data[0];
        const double r_meas = spectr_data[1];

        cmpl ns[nb_med];
        if(fit->run->cache.th_only) {
            fit_engine_get_cached_ns(fit, j, ns);
        } else {
            stack_get_ns_list(fit->stack, ns, lambda);
        }

        /* STEP 3 : We call the procedure to compute the reflectivity. */
        double r_theory;
        mult_layer_refl_sr(nb_med, ns, ths, lambda, fit->acquisition, &r_theory, jacob_th, jacob_n, jacob_acq);

        if(f != NULL) {
            gsl_vector_set(f, j, r_theory - r_meas);
        }

        if(jacob) {
            struct deriv_info * ideriv = fit->run->cache.deriv_info;

            if(! fit->run->cache.th_only) {
                for(int i = 0; i < nb_med; i++) {
                    ideriv[i].is_valid = 0;
                }
            }

            for(int kp = 0; kp < fit->parameters->number; kp++) {
                const fit_param_t *fp = fit->parameters->values + kp;
                double pjac;

                pjac = get_parameter_jacob_r(fp, fit->stack, ideriv, lambda,
                                             jacob_th, jacob_n, jacob_acq);

                gsl_matrix_set(jacob, j, kp, pjac);
            }
        }
    }

    return GSL_SUCCESS;
}

int
refl_fit_f(const gsl_vector *x, void *params, gsl_vector * f)
{
    return refl_fit_fdf(x, params, f, NULL);
}

int
refl_fit_df(const gsl_vector *x, void *params, gsl_matrix *jacob)
{
    return refl_fit_fdf(x, params, NULL, jacob);
}
