#include "elliss-multifit.h"
#include "multi-fit-engine.h"
#include "elliss.h"

/* helper function */
#include "elliss-get-jacob.h"

int
elliss_multifit_fdf(const gsl_vector *x, void *params, gsl_vector *f,
                    gsl_matrix * jacob)
{
    struct multi_fit_engine *fit = params;
    size_t nb_med = fit->stack_list[0]->nb;
    double jacob_th_data[2 * (nb_med - 2)];
    cmpl jacob_n_data[2 * nb_med];
    size_t samples_number = fit->samples_number;
    const enum se_type se_type = GET_SE_TYPE(fit->system_kind);
    size_t j, j_sample, sample;

    /* STEP 1 : We apply the actual values of the fit parameters
                to the stack. */

    multi_fit_engine_commit_parameters(fit, x);

    j_sample = 0;
    for(sample = 0; sample < samples_number; sample++) {
        struct spectrum *spectrum = fit->spectra_list[sample];
        struct stack *stack_sample = fit->stack_list[sample];
        size_t npt = spectra_points(spectrum);
        double jacob_acq_data[2 * SE_ACQ_PARAMETERS_NB(se_type)];

        /* STEP 2 : From the stack we retrive the thicknesses and RIs
        informations. */

        const double *ths = stack_get_ths_list(stack_sample);

        double *jacob_th  = (jacob ? jacob_th_data  : NULL);
        cmpl *  jacob_n   = (jacob ? jacob_n_data   : NULL);
        double *jacob_acq = (jacob ? jacob_acq_data : NULL);

        for(j = 0; j < npt; j++, j_sample++) {
            float const * spectr_data = spectra_get_values(spectrum, j);
            const double lambda     = spectr_data[0];
            const double meas_alpha = spectr_data[1];
            const double meas_beta  = spectr_data[2];
            struct elliss_ab theory[1];

            cmpl ns[nb_med];
            stack_get_ns_list(fit->stack_list[sample], ns, lambda);

            /* STEP 3 : We call the ellipsometer kernel function */

            mult_layer_refl_se(se_type,
                               nb_med, ns, ths, lambda,
                               &fit->acquisitions[sample], theory,
                               jacob_th, jacob_n, jacob_acq);

            if(f != NULL) {
                gsl_vector_set(f, j_sample,       theory->alpha - meas_alpha);
                gsl_vector_set(f, j_sample + npt, theory->beta  - meas_beta);
            }

            if(jacob) {
                struct deriv_info * ideriv = fit->cache.deriv_info;
                const int nb_comm_params = fit->common_parameters->number;
                const int nb_priv_params = fit->private_parameters->number;
                const int nb_params =	nb_comm_params + nb_priv_params * samples_number;
                struct elliss_ab jac[1];

                for(int ic = 0; ic < nb_med; ic++) {
                    ideriv[ic].is_valid = 0;
                }

                int kp;
                for(kp = 0; kp < nb_comm_params; kp++) {
                    fit_param_t const *fp = fit->common_parameters->values + kp;

                    get_parameter_jacobian(fp, fit->stack_list[sample],
                                           ideriv, lambda,
                                           jacob_th, jacob_n, jacob_acq, jac);

                    gsl_matrix_set(jacob, j_sample,       kp, jac->alpha);
                    gsl_matrix_set(jacob, j_sample + npt, kp, jac->beta);
                }

                for(int ikp = 0; ikp < nb_priv_params * sample; ikp++, kp++) {
                    gsl_matrix_set(jacob, j_sample,       kp, 0.0);
                    gsl_matrix_set(jacob, j_sample + npt, kp, 0.0);
                }

                for(int ikp = 0; ikp < nb_priv_params; kp++, ikp++) {
                    fit_param_t *fp = fit->private_parameters->values + ikp;

                    get_parameter_jacobian(fp, fit->stack_list[sample],
                                           ideriv, lambda,
                                           jacob_th, jacob_n, jacob_acq, jac);

                    gsl_matrix_set(jacob, j_sample,       kp, jac->alpha);
                    gsl_matrix_set(jacob, j_sample + npt, kp, jac->beta);
                }

                for(/* */; kp < nb_params; kp++) {
                    gsl_matrix_set(jacob, j_sample,       kp, 0.0);
                    gsl_matrix_set(jacob, j_sample + npt, kp, 0.0);
                }
            }
        }

        j_sample += npt;
    }

    return GSL_SUCCESS;
}

int
elliss_multifit_f(const gsl_vector *x, void *params, gsl_vector * f)
{
    return elliss_multifit_fdf(x, params, f, NULL);
}

int
elliss_multifit_df(const gsl_vector *x, void *params, gsl_matrix *jacob)
{
    return elliss_multifit_fdf(x, params, NULL, jacob);
}
