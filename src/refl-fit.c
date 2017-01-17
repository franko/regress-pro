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
                      double jacob_th[], double jacob_n[], double jacob_acq[1],
                      double rmult, double r_raw)
{
    double result;
    int lyr = fp->layer_nb;

    switch(fp->id) {
        double drdth;
        double dnr, dni;

    case PID_FIRSTMUL:
        result = r_raw;
        break;
    case PID_THICKNESS:
        drdth = jacob_th[lyr - 1];
        result = rmult * drdth;
        break;
    case PID_LAYER_N:
        get_model_param_deriv(stack->disp[lyr], &ideriv[lyr],
                              fp, lambda, &dnr, &dni);

        const double drdn_re = jacob_n[lyr];
        const double drdn_im = jacob_n[stack->nb + lyr];

        result = rmult * (dnr * drdn_re + dni * drdn_im);
        break;
    case PID_BANDWIDTH:
        result = jacob_acq[0];
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
    double jacob_th_data[nb_med - 2], jacob_n_data[2 * nb_med];
    double jacob_acq[1]; /* BANDWIDTH is the only acquisition parameter. */

    /* STEP 1 : We apply the actual values of the fit parameters
                to the stack. */

    fit_engine_commit_parameters(fit, x);

    /* STEP 2 : From the stack we retrive the thicknesses and RIs
                informations. */

    const double *ths = stack_get_ths_list(fit->stack);

    double *jacob_th = (jacob ? jacob_th_data : NULL);
    double *jacob_n  = (jacob && !fit->run->th_only ? jacob_n_data  : NULL);

    int parameters_number_sum = 0;
    for(int i = 0; i < nb_med; i++) {
        parameters_number_sum += disp_get_number_of_params(fit->stack->disp[i]);
    }

    struct deriv_info ideriv[nb_med];
    cmpl ideriv_data[parameters_number_sum];
    if(!fit->run->th_only) {
        int data_offset = 0;
        for(int i = 0; i < nb_med; i++) {
            int parameters_number = disp_get_number_of_params(fit->stack->disp[i]);
            ideriv[i].is_valid = 0;
            ideriv[i].data = ideriv_data + data_offset;
            ideriv[i].parameters_number = parameters_number;
            data_offset += parameters_number;
        }
    }

    const double rmult = acquisition_get_parameter(fit->acquisition, PID_FIRSTMUL);
    for(int j = 0; j < spectra_points(s); j++) {
        float const * spectr_data = spectra_get_values(s, j);
        const double lambda = spectr_data[0];
        const double r_meas = spectr_data[1];

        cmpl ns[nb_med];
        stack_get_ns_list(fit->stack, ns, lambda);

        /* STEP 3 : We call the procedure to compute the reflectivity. */
        const double r_raw = mult_layer_refl_sr(nb_med, ns, ths, lambda, fit->acquisition, jacob_th, jacob_n, jacob_acq);

        const double r_theory = rmult * r_raw;

        if(f != NULL) {
            gsl_vector_set(f, j, r_theory - r_meas);
        }

        if(jacob) {
            size_t kp, ic;

            if(! fit->run->th_only) {
                for(ic = 0; ic < nb_med; ic++) {
                    ideriv[ic].is_valid = 0;
                }
            }

            for(kp = 0; kp < fit->parameters->number; kp++) {
                const fit_param_t *fp = fit->parameters->values + kp;
                double pjac;

                pjac = get_parameter_jacob_r(fp, fit->stack, ideriv, lambda,
                                             jacob_th, jacob_n, jacob_acq,
                                             rmult, r_raw);

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
