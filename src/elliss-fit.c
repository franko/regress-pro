#include "elliss-fit.h"
#include "fit-engine.h"
#include "elliss.h"

/* helper function */
#include "elliss-get-jacob.h"

void
get_parameter_jacobian(fit_param_t const *fp, stack_t const *stack,
                       struct deriv_info *ideriv, double lambda,
                       gsl_vector *stack_jacob_th, cmpl_vector *stack_jacob_n,
                       double *jacob_acquisition, struct elliss_ab *result)
{
    const int nb_med = stack->nb, nb_lyr = nb_med - 2;
    const int layer = fp->layer_nb;
    struct {
        cmpl alpha, beta;
    } drdn;

    switch(fp->id) {
        double dnr, dni;
    case PID_THICKNESS:

        result->alpha = gsl_vector_get(stack_jacob_th, layer - 1);
        result->beta  = gsl_vector_get(stack_jacob_th, nb_lyr + layer - 1);
        break;
    case PID_LAYER_N:
        get_model_param_deriv(stack->disp[layer],
                              &ideriv[layer], fp, lambda,
                              &dnr, &dni);

        drdn.alpha = cmpl_vector_get(stack_jacob_n, layer);
        drdn.beta  = cmpl_vector_get(stack_jacob_n, nb_med + layer);

        result->alpha = creal(drdn.alpha)*dnr - cimag(drdn.alpha)*dni;
        result->beta  = creal(drdn.beta) *dnr - cimag(drdn.beta) *dni;
        break;
    case PID_AOI:
        result->alpha = jacob_acquisition[SE_ACQ_INDEX(SE_ALPHA, SE_AOI)];
        result->beta  = jacob_acquisition[SE_ACQ_INDEX(SE_BETA , SE_AOI)];
        break;
    case PID_ANALYZER:
        result->alpha = jacob_acquisition[SE_ACQ_INDEX(SE_ALPHA, SE_ANALYZER)];
        result->beta  = jacob_acquisition[SE_ACQ_INDEX(SE_BETA , SE_ANALYZER)];
        break;
    case PID_NUMAP:
        result->alpha = jacob_acquisition[SE_ACQ_INDEX(SE_ALPHA, SE_NUMAP)];
        result->beta  = jacob_acquisition[SE_ACQ_INDEX(SE_BETA , SE_NUMAP)];
        break;
    case PID_BANDWIDTH:
        result->alpha = jacob_acquisition[SE_ACQ_INDEX(SE_ALPHA, SE_BANDWIDTH)];
        result->beta  = jacob_acquisition[SE_ACQ_INDEX(SE_BETA , SE_BANDWIDTH)];
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
    struct fit_engine *fit = params;
    struct spectrum *s = fit->run->spectr;
    const int nb_med = fit->stack->nb;
    const int npt = spectra_points(s);
    struct {
        double const * ths;
        cmpl * ns;
    } actual;
    struct {
        gsl_vector *th;
        cmpl_vector *n;
    } wjacob;
    const enum se_type se_type = GET_SE_TYPE(fit->acquisition->type);
    double jacob_acq_data[2 * SE_ACQ_PARAMETERS_NB(se_type)];

    /* STEP 1 : We apply the actual values of the fit parameters
                to the stack. */

    fit_engine_commit_parameters(fit, x);

    /* STEP 2 : From the stack we retrive the thicknesses and RIs
                informations. */

    actual.ths = stack_get_ths_list(fit->stack);

    wjacob.th = (jacob ? fit->run->jac_th : NULL);
    wjacob.n  = (jacob && !fit->run->cache.th_only ? fit->run->jac_n.ell : NULL);
    double *jacob_acquisition = (jacob && fit->run->cache.require_acquisition_jacob ? jacob_acq_data : NULL);

    for(int j = 0; j < npt; j++) {
        float const * spectr_data = spectra_get_values(s, j);
        const double lambda     = spectr_data[0];
        const double meas_alpha = spectr_data[1];
        const double meas_beta  = spectr_data[2];
        struct elliss_ab theory[1];

        if(fit->run->cache.th_only) {
            actual.ns = fit->run->cache.ns_full_spectr + j * nb_med;
        } else {
            actual.ns = fit->run->cache.ns;
            stack_get_ns_list(fit->stack, actual.ns, lambda);
        }

        /* STEP 3 : We call the ellipsometer kernel function */

        mult_layer_refl_se(se_type, nb_med, actual.ns, actual.ths, lambda,
            fit->acquisition, theory, wjacob.th, wjacob.n, jacob_acquisition);

        if(f != NULL) {
            gsl_vector_set(f, j,       theory->alpha - meas_alpha);
            gsl_vector_set(f, npt + j, theory->beta  - meas_beta);
        }

        if(jacob) {
            struct deriv_info * ideriv = fit->run->cache.deriv_info;
            struct elliss_ab jac[1];

            if(! fit->run->cache.th_only) {
                for(int ic = 0; ic < nb_med; ic++) {
                    ideriv[ic].is_valid = 0;
                }
            }

            for(int kp = 0; kp < fit->parameters->number; kp++) {
                fit_param_t *fp = fit->parameters->values + kp;

                get_parameter_jacobian(fp, fit->stack, ideriv, lambda,
                                       wjacob.th, wjacob.n, jacob_acquisition, jac);

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
    return elliss_fit_fdf(x, params, f, NULL);
}

int
elliss_fit_df(const gsl_vector *x, void *params, gsl_matrix *jacob)
{
    return elliss_fit_fdf(x, params, NULL, jacob);
}
