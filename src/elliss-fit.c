#include "elliss-fit.h"
#include "fit-engine.h"
#include "elliss.h"

/* helper function */
#include "elliss-get-jacob.h"

static double deg_to_radians(double x) { return x * M_PI / 180.0; }

void
get_parameter_jacobian(fit_param_t const *fp, stack_t const *stack,
                       struct deriv_info *ideriv, double lambda, double aoi_der[2],
                       gsl_vector *stack_jacob_th, cmpl_vector *stack_jacob_n,
                       struct elliss_ab *result)
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
        result->alpha = aoi_der[0];
        result->beta = aoi_der[1];
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
    size_t nb_med = fit->stack->nb;
    struct {
        double const * ths;
        cmpl * ns;
    } actual;
    struct {
        gsl_vector *th;
        cmpl_vector *n;
    } wjacob;
    size_t npt = spectra_points(s);
    const enum se_type se_type = GET_SE_TYPE(fit->acquisition->type);
    double aoi_der_data[2], *aoi_der;
    size_t j;

    /* STEP 1 : We apply the actual values of the fit parameters
                to the stack. */

    fit_engine_commit_parameters(fit, x);

    /* STEP 2 : From the stack we retrive the thicknesses and RIs
                informations. */

    actual.ths = stack_get_ths_list(fit->stack);

    wjacob.th = (jacob ? fit->run->jac_th : NULL);
    wjacob.n  = (jacob && !fit->run->cache.th_only ? fit->run->jac_n.ell : NULL);
    aoi_der = (jacob && fit->run->cache.need_aoi_derivative ? aoi_der_data : NULL);

    const double phi0 = deg_to_radians(acquisition_get_parameter(fit->acquisition, PID_AOI));
    const double anlz = deg_to_radians(acquisition_get_parameter(fit->acquisition, PID_ANALYZER));
    for(j = 0; j < npt; j++) {
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


        mult_layer_se_jacob(se_type,
                            nb_med, actual.ns, phi0, actual.ths, lambda,
                            anlz, theory, aoi_der, wjacob.th, wjacob.n);

        if(f != NULL) {
            gsl_vector_set(f, j,       theory->alpha - meas_alpha);
            gsl_vector_set(f, npt + j, theory->beta  - meas_beta);
        }

        if(jacob) {
            struct deriv_info * ideriv = fit->run->cache.deriv_info;
            struct elliss_ab jac[1];
            size_t kp, ic;

            if(! fit->run->cache.th_only) {
                for(ic = 0; ic < nb_med; ic++) {
                    ideriv[ic].is_valid = 0;
                }
            }

            for(kp = 0; kp < fit->parameters->number; kp++) {
                fit_param_t *fp = fit->parameters->values + kp;

                get_parameter_jacobian(fp, fit->stack, ideriv, lambda,
                                       aoi_der, wjacob.th, wjacob.n, jac);

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
