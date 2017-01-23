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
    double jacob_th_data[nb_med - 2], jacob_n_data[2 * nb_med];
    double jacob_acq[1]; /* BANDWIDTH is the only acquisition parameter. */
    size_t sample;
    size_t j, j_sample;

    /* STEP 1 : We apply the actual values of the fit parameters
                to the stack. */

    multi_fit_engine_commit_parameters(fit, x);

    j_sample = 0;
    for(sample = 0; sample < samples_number; sample++) {
        struct spectrum *spectrum = fit->spectra_list[sample];
        struct stack *stack_sample = fit->stack_list[sample];

        /* STEP 2 : From the stack we retrive the thicknesses and RIs
        informations. */

        const double *ths = stack_get_ths_list(fit->stack_list[sample]);

        double *jacob_th = (jacob ? jacob_th_data : NULL);
        double *jacob_n  = (jacob ? jacob_n_data  : NULL);

        for(j = 0; j < spectra_points(spectrum); j++, j_sample++) {
            float const * spectr_data = spectra_get_values(spectrum, j);
            const double lambda = spectr_data[0];
            const double r_meas = spectr_data[1];
            double r_raw, r_theory;
            double rmult = acquisition_get_parameter(&fit->acquisitions[sample], PID_FIRSTMUL);
            const size_t nb_priv_params = fit->private_parameters->number;

            cmpl ns[nb_med];
            stack_get_ns_list(fit->stack_list[sample], ns, lambda);

            /* STEP 3 : We call the procedure to compute the reflectivity. */
            r_raw = mult_layer_refl_sr(nb_med, ns, ths, lambda,
                                       &fit->acquisitions[sample], jacob_th, jacob_n, jacob_acq);

            r_theory = rmult * r_raw;

            if(f != NULL) {
                gsl_vector_set(f, j_sample, r_theory - r_meas);
            }

            if(jacob) {
                struct deriv_info * ideriv = fit->cache.deriv_info;
                int nb_params = fit->common_parameters->number + fit->private_parameters->number * samples_number;

                for (int ic = 0; ic < nb_med; ic++) {
                    ideriv[ic].is_valid = 0;
                }

                int kp;
                for (kp = 0; kp < fit->common_parameters->number; kp++) {
                    fit_param_t *fp = fit->common_parameters->values + kp;
                    double pjac;

                    pjac = get_parameter_jacob_r(fp, fit->stack_list[sample],
                                                 ideriv, lambda,
                                                 jacob_th, jacob_n, jacob_acq,
                                                 rmult, r_raw);

                    gsl_matrix_set(jacob, j_sample, kp, pjac);
                }

                for(int ikp = 0; ikp < nb_priv_params * sample; kp++, ikp++) {
                    gsl_matrix_set(jacob, j_sample, kp, 0.0);
                }

                for(int ikp = 0; ikp < nb_priv_params; kp++, ikp++) {
                    fit_param_t *fp = fit->private_parameters->values + ikp;
                    double pjac;

                    pjac = get_parameter_jacob_r(fp, fit->stack_list[sample],
                                                 ideriv, lambda,
                                                 jacob_th, jacob_n, jacob_acq,
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
