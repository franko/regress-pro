
/*
  $Id$
*/

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
    struct {
        double const * ths;
        cmpl * ns;
    } actual;
    struct {
        gsl_vector *th;
        cmpl_vector *n;
    } stack_jacob;
    size_t samples_number = fit->samples_number;
    const enum se_type se_type = GET_SE_TYPE(fit->system_kind);
    size_t j, j_sample, sample;

    /* STEP 1 : We apply the actual values of the fit parameters
                to the stack. */

    multi_fit_engine_commit_parameters(fit, x);

    j_sample = 0;
    for(sample = 0; sample < samples_number; sample++) {
        struct spectrum *spectrum = fit->spectra_list[sample];
        size_t npt = spectra_points(spectrum);

        /* STEP 2 : From the stack we retrive the thicknesses and RIs
        informations. */

        actual.ths = stack_get_ths_list(fit->stack_list[sample]);

        stack_jacob.th = (jacob ? fit->jac_th    : NULL);
        stack_jacob.n  = (jacob ? fit->jac_n.ell : NULL);

        for(j = 0; j < npt; j++, j_sample++) {
            float const * spectr_data = spectra_get_values(spectrum, j);
            const double lambda     = spectr_data[0];
            const double meas_alpha = spectr_data[1];
            const double meas_beta  = spectr_data[2];
            const double phi0 = spectrum->config.aoi;
            const double anlz = spectrum->config.analyzer;
            struct elliss_ab theory[1];

            actual.ns = fit->cache.ns;
            stack_get_ns_list(fit->stack_list[sample], actual.ns, lambda);

            /* STEP 3 : We call the ellipsometer kernel function */

            mult_layer_se_jacob(se_type,
                                nb_med, actual.ns, phi0, actual.ths, lambda,
                                anlz, theory, stack_jacob.th, stack_jacob.n);

            if(f != NULL) {
                gsl_vector_set(f, j_sample,       theory->alpha - meas_alpha);
                gsl_vector_set(f, j_sample + npt, theory->beta  - meas_beta);
            }

            if(jacob) {
                struct deriv_info * ideriv = fit->cache.deriv_info;
                const size_t nb_comm_params = fit->common_parameters->number;
                const size_t nb_priv_params = fit->private_parameters->number;
                size_t nb_params =					\
                                                    nb_comm_params + nb_priv_params * samples_number;
                struct elliss_ab jac[1];
                size_t kp, ikp, ic;

                for(ic = 0; ic < nb_med; ic++) {
                    ideriv[ic].is_valid = 0;
                }

                for(kp = 0; kp < nb_comm_params; kp++) {
                    fit_param_t const *fp = fit->common_parameters->values + kp;

                    get_parameter_jacobian(fp, fit->stack_list[sample],
                                           ideriv, lambda,
                                           stack_jacob.th, stack_jacob.n,
                                           jac);

                    gsl_matrix_set(jacob, j_sample,       kp, jac->alpha);
                    gsl_matrix_set(jacob, j_sample + npt, kp, jac->beta);
                }

                for(ikp = 0; ikp < nb_priv_params * sample; ikp++, kp++) {
                    gsl_matrix_set(jacob, j_sample,       kp, 0.0);
                    gsl_matrix_set(jacob, j_sample + npt, kp, 0.0);
                }

                for(ikp = 0; ikp < nb_priv_params; kp++, ikp++) {
                    fit_param_t *fp = fit->private_parameters->values + ikp;

                    get_parameter_jacobian(fp, fit->stack_list[sample],
                                           ideriv, lambda,
                                           stack_jacob.th, stack_jacob.n,
                                           jac);

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
