#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <assert.h>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

#include "refl-kernel.h"
#include "refl-multifit.h"
#include "multi-fit-engine.h"
#include "refl-get-jacobian.h"

int
refl_multifit_fdf(const gsl_vector *x, void *params,
                  gsl_vector *f, gsl_matrix * jacob)
{
    struct multi_fit_engine *fit = params;
    size_t nb_med = fit->stack_list[0]->nb;
    size_t samples_number = fit->samples_number;
    struct {
        double const * ths;
        cmpl * ns;
    } actual;
    gsl_vector *r_th_jacob, *r_n_jacob;
    size_t sample;
    size_t j, j_sample;

    /* STEP 1 : We apply the actual values of the fit parameters
                to the stack. */

    multi_fit_engine_commit_parameters(fit, x);

    j_sample = 0;
    for(sample = 0; sample < samples_number; sample++) {
        struct spectrum *spectrum = fit->spectra_list[sample];

        /* STEP 2 : From the stack we retrive the thicknesses and RIs
        informations. */

        actual.ths = stack_get_ths_list(fit->stack_list[sample]);

        r_th_jacob = (jacob ? fit->jac_th : NULL);
        r_n_jacob  = (jacob ? fit->jac_n.refl : NULL);

        for(j = 0; j < spectra_points(spectrum); j++, j_sample++) {
            float const * spectr_data = spectra_get_values(spectrum, j);
            const double lambda = spectr_data[0];
            const double r_meas = spectr_data[1];
            double r_raw, r_theory;
            double rmult = acquisition_get_parameter(&fit->acquisitions[sample], PID_FIRSTMUL);
            const size_t nb_priv_params = fit->private_parameters->number;

            actual.ns = fit->cache.ns;
            stack_get_ns_list(fit->stack_list[sample], actual.ns, lambda);

            /* STEP 3 : We call the procedure mult_layer_refl_ni */

            r_raw = mult_layer_refl_ni(nb_med, actual.ns, actual.ths, lambda,
                                       r_th_jacob, r_n_jacob);

            r_theory = rmult * r_raw;

            if(f != NULL) {
                gsl_vector_set(f, j_sample, r_theory - r_meas);
            }

            if(jacob) {
                size_t kp, ikp, ic;
                size_t nb_params =					\
                                                    fit->common_parameters->number + \
                                                    fit->private_parameters->number * samples_number;
                struct deriv_info * ideriv = fit->cache.deriv_info;

                for(ic = 0; ic < nb_med; ic++) {
                    ideriv[ic].is_valid = 0;
                }

                for(kp = 0; kp < fit->common_parameters->number; kp++) {
                    fit_param_t *fp = fit->common_parameters->values + kp;
                    double pjac;

                    pjac = get_parameter_jacob_r(fp, fit->stack_list[sample],
                                                 ideriv, lambda,
                                                 r_th_jacob, r_n_jacob,
                                                 rmult, r_raw);

                    gsl_matrix_set(jacob, j_sample, kp, pjac);
                }

                for(ikp = 0; ikp < nb_priv_params * sample; kp++, ikp++) {
                    gsl_matrix_set(jacob, j_sample, kp, 0.0);
                }

                for(ikp = 0; ikp < nb_priv_params; kp++, ikp++) {
                    fit_param_t *fp = fit->private_parameters->values + ikp;
                    double pjac;

                    pjac = get_parameter_jacob_r(fp, fit->stack_list[sample],
                                                 ideriv, lambda,
                                                 r_th_jacob, r_n_jacob,
                                                 rmult, r_raw);

                    gsl_matrix_set(jacob, j_sample, kp, pjac);
                }

                for(/* */; kp < nb_params; kp++) {
                    gsl_matrix_set(jacob, j_sample, kp, 0.0);
                }
            }
        }
    }

    return GSL_SUCCESS;
}

int
refl_multifit_f(const gsl_vector *x, void *params, gsl_vector * f)
{
    return refl_multifit_fdf(x, params, f, NULL);
}

int
refl_multifit_df(const gsl_vector *x, void *params, gsl_matrix *jacob)
{
    return refl_multifit_fdf(x, params, NULL, jacob);
}
