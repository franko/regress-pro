#include "elliss-fit.h"
#include "fit-engine.h"
#include "elliss.h"

/* helper function */
#include "elliss-get-jacob.h"

void
get_parameter_jacobian(fit_param_t const *fp, stack_t const *stack,
                       struct deriv_info *ideriv, double lambda,
                       double_array jacob_th, cmpl_array jacob_n,
                       double_array jacob_acq, struct elliss_ab *result)
{
    const int nb_med = stack->nb, nb_lyr = nb_med - 2;
    const int layer = fp->layer_nb;
    double dnr, dni;

    switch(fp->id) {
    case PID_THICKNESS:
        result->alpha = jacob_th[layer - 1];
        result->beta  = jacob_th[nb_lyr + layer - 1];
        break;
    case PID_LAYER_N:
        {
            get_model_param_deriv(stack->disp[layer],
                                  &ideriv[layer], fp, lambda,
                                  &dnr, &dni);
            cmpl drdn_alpha = jacob_n[layer];
            cmpl drdn_beta  = jacob_n[nb_med + layer];

            result->alpha = std::real(drdn_alpha)*dnr - std::imag(drdn_alpha)*dni;
            result->beta  = std::real(drdn_beta) *dnr - std::imag(drdn_beta) *dni;
            break;
        }
    case PID_AOI:
        result->alpha = jacob_acq[SE_ACQ_INDEX(SE_ALPHA, SE_AOI)];
        result->beta  = jacob_acq[SE_ACQ_INDEX(SE_BETA , SE_AOI)];
        break;
    case PID_ANALYZER:
    case PID_POLARIZER:
        result->alpha = jacob_acq[SE_ACQ_INDEX(SE_ALPHA, SE_ANALYZER)];
        result->beta  = jacob_acq[SE_ACQ_INDEX(SE_BETA , SE_ANALYZER)];
        break;
    case PID_NUMAP:
        result->alpha = jacob_acq[SE_ACQ_INDEX(SE_ALPHA, SE_NUMAP)];
        result->beta  = jacob_acq[SE_ACQ_INDEX(SE_BETA , SE_NUMAP)];
        break;
    case PID_BANDWIDTH:
        result->alpha = jacob_acq[SE_ACQ_INDEX(SE_ALPHA, SE_BANDWIDTH)];
        result->beta  = jacob_acq[SE_ACQ_INDEX(SE_BETA , SE_BANDWIDTH)];
        break;
    default:
        result->alpha = 0.0;
        result->beta = 0.0;
    }
}

int
elliss_fit_fdf(const gsl_vector *x, void *params, gsl_vector *f,
               gsl_matrix * jacob)
{
    struct fit_engine *fit = (struct fit_engine *) params;
    struct spectrum *s = fit->run->spectr;
    const int nb_med = fit->stack->nb;
    const int npt = spectra_points(s);
    double_array16 jacob_th(2 * (nb_med - 2));
    cmpl_array16 jacob_n(2 * nb_med);
    const enum se_type se_type = SE_TYPE(fit->acquisition->type);
    double_array8 jacob_acq(2 * SE_ACQ_PARAMETERS_NB(se_type));

    /* STEP 1 : We apply the actual values of the fit parameters
                to the stack. */

    fit_engine_commit_parameters(fit, x);

    /* STEP 2 : From the stack we retrive the thicknesses and RIs
                informations. */

    const double *ths = stack_get_ths_list(fit->stack);

    double_array *jacob_th_ptr  = (jacob ? &jacob_th : nullptr);
    cmpl_array   *jacob_n_ptr   = (jacob && !fit->run->cache.th_only ? &jacob_n : nullptr);
    double_array *jacob_acq_ptr = (jacob && fit->run->cache.require_acquisition_jacob ? &jacob_acq : nullptr);

    for(int j = 0; j < npt; j++) {
        float const * spectr_data = spectra_get_values(s, j);
        const double lambda     = spectr_data[0];
        const double meas_alpha = spectr_data[1];
        const double meas_beta  = spectr_data[2];
        struct elliss_ab theory[1];

        cmpl_array8 ns(nb_med);
        if(fit->run->cache.th_only) {
            fit_engine_get_cached_ns(fit, j, ns);
        } else {
            stack_get_ns_list(fit->stack, ns, lambda);
        }

        /* STEP 3 : We call the ellipsometer kernel function */

        mult_layer_refl_se(se_type, nb_med, ns, ths, lambda,
            fit->acquisition, theory, jacob_th_ptr, jacob_n_ptr, jacob_acq_ptr);

        if(f != nullptr) {
            gsl_vector_set(f, j,       theory->alpha - meas_alpha);
            gsl_vector_set(f, npt + j, theory->beta  - meas_beta);
        }

        if(jacob) {
            struct deriv_info *ideriv = fit->run->cache.deriv_info;
            struct elliss_ab jac[1];

            if(!fit->run->cache.th_only) {
                for(int i = 0; i < nb_med; i++) {
                    ideriv[i].is_valid = 0;
                }
            }

            for(int kp = 0; kp < (int) fit->parameters->number; kp++) {
                fit_param_t *fp = &fit->parameters->at(kp);

                get_parameter_jacobian(fp, fit->stack, ideriv, lambda,
                                       jacob_th, jacob_n, jacob_acq, jac);

                gsl_matrix_set(jacob, j,       kp, jac->alpha);
                gsl_matrix_set(jacob, npt + j, kp, jac->beta);
            }
        }
    }

    return GSL_SUCCESS;
}

int
elliss_fit_f(const gsl_vector *x, void *params, gsl_vector * f)
{
    return elliss_fit_fdf(x, params, f, nullptr);
}

int
elliss_fit_df(const gsl_vector *x, void *params, gsl_matrix *jacob)
{
    return elliss_fit_fdf(x, params, nullptr, jacob);
}
