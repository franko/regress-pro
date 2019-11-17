
#include <assert.h>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>

#include "disp-fit-engine.h"
#include "lmfit.h"
#include "vector_print.h"
#include "str.h"
#include "fit-timestamp.h"

#ifdef DEBUG_DISP_DERIV
#include "disp-deriv-check.h"
#endif

static int disp_fit_fdf(const gsl_vector *x, void *_fit, gsl_vector *f,
                        gsl_matrix * jacob);

void
disp_fit_config_init(struct disp_fit_config *cfg)
{
    cfg->nb_max_iters = 20;
    cfg->chisq_norm_factor = 1.0;
    cfg->epsabs = 1e-6;
    cfg->epsrel = 1e-6;
}

void
disp_fit_engine_free(struct disp_fit_engine *fit)
{
    if(fit->wl) {
        gsl_vector_free(fit->wl);
    }

    if(fit->model_disp) {
        disp_free(fit->model_disp);
    }
    if(fit->ref_disp) {
        disp_free(fit->ref_disp);
    }

    free(fit);
}

struct disp_fit_engine *
disp_fit_engine_new() {
    struct disp_fit_engine *fit = (struct disp_fit_engine *) emalloc(sizeof(struct disp_fit_engine));
    fit->ref_disp   = nullptr;
    fit->model_disp = nullptr;
    fit->model_der  = nullptr;
    fit->wl         = nullptr;
    fit->parameters = nullptr;
    return fit;
}

void
disp_fit_engine_set_parameters(struct disp_fit_engine *fit,
                               const fit_parameters *fps)
{
    fit->parameters = fps;
    assert(fps->number > 0);
}

static void
commit_fit_parameters(struct disp_fit_engine *fit, const gsl_vector *x)
{
    for(int k = 0; k < fit->parameters->number; k++) {
        const fit_param_t& fp = fit->parameters->at(k);
        double value = gsl_vector_get(x, k);
        dispers_apply_param(fit->model_disp, &fp, value);
    }
}

static void
update_disp_info(struct disp_fit_engine *fit)
{
    double wavelength_start = gsl_vector_get(fit->wl, 0);
    double wavelength_end   = gsl_vector_get(fit->wl, fit->wl->size - 1);
    disp_set_info_wavelength(fit->model_disp, wavelength_start, wavelength_end);
    char buffer[128];
    if (fit_timestamp(128, buffer) != 0) {
        disp_set_modifications_flag(fit->model_disp, buffer);
    }
}

int
disp_fit_fdf(const gsl_vector *x, void *_fit, gsl_vector *f,
             gsl_matrix * jacob)
{
    struct disp_fit_engine *fit = (struct disp_fit_engine *) _fit;
    gsl_vector *wl = fit->wl;
    size_t nsmp = wl->size, j;

    commit_fit_parameters(fit, x);

    for(j = 0; j < nsmp; j++) {
        double lambda = gsl_vector_get(wl, j);
        const fit_parameters *params = fit->parameters;

        if(f) {
            cmpl n_mod = n_value(fit->model_disp, lambda);
            cmpl n_ref = n_value(fit->ref_disp, lambda);

            double nrdiff = std::real(n_mod) - std::real(n_ref);
            double nidiff = std::imag(n_mod) - std::imag(n_ref);

            gsl_vector_set(f, j,      nrdiff);
            gsl_vector_set(f, j+nsmp, nidiff);
        }

        if(jacob) {
            n_value_deriv(fit->model_disp, fit->model_der, lambda);

            for (int kp = 0; kp < fit->parameters->number; kp++) {
                cmpl dndp = fit->model_der->at(params->at(kp).param_nb);

                gsl_matrix_set(jacob, j,      kp, std::real(dndp));
                gsl_matrix_set(jacob, j+nsmp, kp, std::imag(dndp));
            }
        }
    }

    return GSL_SUCCESS;
}

static int
disp_fit_df(const gsl_vector *x, void *_fit, gsl_matrix *j)
{
    return disp_fit_fdf(x, _fit, nullptr, j);
}

static int
disp_fit_f(const gsl_vector *x, void *_fit, gsl_vector *f)
{
    return disp_fit_fdf(x, _fit, f, nullptr);
}

#ifdef DEBUG_DISP_DERIV
static void check_disp_derivatives(disp_fit_engine *fit) {
    for (unsigned k = 0; k < fit->wl->size; k++) {
        const double wavelength = gsl_vector_get(fit->wl, k);
        disp_deriv_check(fit->model_disp, wavelength);
    }
}
#endif

int
lmfit_disp(struct disp_fit_engine *fit, struct disp_fit_config *cfg,
           gsl_vector *x, struct lmfit_result *result, str_ptr analysis, str_ptr error_msg)
{
    const gsl_multifit_fdfsolver_type *T;
    gsl_multifit_fdfsolver *s;
    gsl_multifit_function_fdf f;
    int disp_nb_params;
    size_t nsp, nfp;
    double chi;
    int status;
    int iter;

    if(!fit->wl || !fit->model_disp || !fit->ref_disp) {
        return 1;
    }

    nfp = fit->parameters->number;
    nsp = fit->wl->size;

    disp_nb_params = disp_get_number_of_params(fit->model_disp);

    assert(fit->model_der == nullptr);
    fit->model_der = new cmpl_vector(disp_nb_params);

    f.f      = & disp_fit_f;
    f.df     = & disp_fit_df;
    f.fdf    = & disp_fit_fdf;
    f.n      = 2 * nsp;
    f.p      = nfp;
    f.params = fit;

#ifdef DEBUG_DISP_DERIV
    fprintf(stderr, "checking dispersions derivatives...\n");
    check_disp_derivatives(fit);
    fprintf(stderr, "done.\n");
#endif

    /* We choose Levenberg-Marquardt algorithm, scaled version*/
    T = gsl_multifit_fdfsolver_lmsder;
    s = gsl_multifit_fdfsolver_alloc(T, f.n, f.p);

    if(analysis) {
        str_copy_c(analysis, "Seed used: ");
        print_vector(analysis, "%.5g", x);
    }

    status = lmfit_iter(x, &f, s, cfg->nb_max_iters, cfg->epsabs, cfg->epsrel,
                        & iter, nullptr, nullptr, nullptr);

    chi = gsl_blas_dnrm2(s->f);
    result->chisq = cfg->chisq_norm_factor * pow(chi, 2.0) / f.n;
    result->nb_points = nsp;
    result->nb_iterations = iter;
    result->gsl_status = status;

    if(error_msg) {
        switch(status) {
        case GSL_SUCCESS:
            str_trunc(error_msg, 0);
            break;
        case GSL_CONTINUE:
            str_copy_c(error_msg, "Error: more iterations needed.");
            break;
        default:
            str_printf(error_msg, "Error: %s", gsl_strerror(status));
        }
    }

    if(analysis) {
        str_printf_add(analysis, "Nb of iterations to converge: %i\n", iter);
    }

    commit_fit_parameters(fit, x);
    update_disp_info(fit);

    delete fit->model_der;
    fit->model_der = nullptr;

    gsl_multifit_fdfsolver_free(s);

    return status;
}
