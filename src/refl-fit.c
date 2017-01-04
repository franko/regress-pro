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
                      gsl_vector *stack_jacob_th,
                      gsl_vector *stack_jacob_n,
                      double rmult, double r_raw)
{
    double result;
    int lyr = fp->layer_nb;

    switch(fp->id) {
        double drdth;
        struct {
            double re, im;
        } drdn;
        double dnr, dni;

    case PID_FIRSTMUL:
        result = r_raw;
        break;
    case PID_THICKNESS:
        drdth = gsl_vector_get(stack_jacob_th, lyr - 1);
        result = rmult * drdth;
        break;
    case PID_LAYER_N:
        get_model_param_deriv(stack->disp[lyr], &ideriv[lyr],
                              fp, lambda, &dnr, &dni);

        drdn.re = gsl_vector_get(stack_jacob_n, lyr);
        drdn.im = gsl_vector_get(stack_jacob_n, stack->nb + lyr);

        result = rmult * (dnr * drdn.re + dni * drdn.im);
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
    gsl_vector *r_th_jacob, *r_n_jacob;
    double const * ths;
    cmpl * ns;
    size_t j;

    /* STEP 1 : We apply the actual values of the fit parameters
                to the stack. */

    fit_engine_commit_parameters(fit, x);

    /* STEP 2 : From the stack we retrive the thicknesses and RIs
                informations. */

    ths = stack_get_ths_list(fit->stack);

    r_th_jacob = (jacob ? fit->run->jac_th : NULL);
    r_n_jacob  = (jacob ? fit->run->jac_n.refl : NULL);

    const double rmult = acquisition_get_parameter(fit->acquisition, PID_FIRSTMUL);
    for(j = 0; j < spectra_points(s); j++) {
        float const * spectr_data = spectra_get_values(s, j);
        const double lambda = spectr_data[0];
        const double r_meas = spectr_data[1];

        if(fit->run->cache.th_only) {
            ns = fit->run->cache.ns_full_spectr + j * nb_med;
        } else {
            ns = fit->run->cache.ns;
            stack_get_ns_list(fit->stack, ns, lambda);
        }

        /* STEP 3 : We call the procedure to compute the reflectivity. */
        const double r_raw = mult_layer_refl_sr(nb_med, ns, ths, lambda, fit->acquisition, r_th_jacob, r_n_jacob);

        const double r_theory = rmult * r_raw;

        if(f != NULL) {
            gsl_vector_set(f, j, r_theory - r_meas);
        }

        if(jacob) {
            size_t kp, ic;
            struct deriv_info * ideriv = fit->run->cache.deriv_info;

            if(! fit->run->cache.th_only) {
                for(ic = 0; ic < nb_med; ic++) {
                    ideriv[ic].is_valid = 0;
                }
            }

            for(kp = 0; kp < fit->parameters->number; kp++) {
                const fit_param_t *fp = fit->parameters->values + kp;
                double pjac;

                pjac = get_parameter_jacob_r(fp, fit->stack, ideriv, lambda,
                                             r_th_jacob, r_n_jacob,
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
