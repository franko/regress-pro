#include <nlopt.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>

#include "math-utils.h"
#include "nlopt_fit.h"
#include "fit_result.h"
#include "gsl_cpp.h"
#include "cmpl.h"

enum {
    MY_NLOPT_MAXEVAL = 5000,
    MY_NLOPT_MAXEVAL_FINAL = 500,
    MY_NLOPT_EVAL_REFRESH = 50,
    MY_NLOPT_MAX_SECONDS = 8,
};

enum {
    GSEARCH_SUCCESS = 0,
    GSEARCH_STOP_REQUEST,
    GSEARCH_FAILURE,
};

// Used for the procedure that set optimizer bounds based on fit parameters.
// The large options set the limits to +/- inf for parameters without a given range.
enum { OPT_BOUNDS_RESTRICT, OPT_BOUNDS_LARGE };

// To be used as the user data in the nlopt ojective function.
// Hold the resources to evaluate the f and df using the fit_engine's
// multifit fdf function.
struct objective_data {
    fit_engine *fit;
    gsl_vector *f;
    gsl_matrix *J;
    nlopt_opt opt; // Hold just a reference to the optimizer. Needed to call nlopt_force_stop
                   // from objective function.
    gui_hook_func_t hfun;
    void *hdata;
    int eval_count;
    int max_eval;

    objective_data(fit_engine *_fit, nlopt_opt _opt, gui_hook_func_t _hfun, void *_hdata, int _max_eval):
        fit(_fit), opt(_opt), hfun(_hfun), hdata(_hdata), eval_count(0), max_eval(_max_eval) {
        f = gsl_vector_alloc(fit->run->mffun.n);
        J = gsl_matrix_alloc(fit->run->mffun.n, fit->run->mffun.p);
    }

    ~objective_data() {
        gsl_vector_free(f);
        gsl_matrix_free(J);
    }

    bool check_for_stop_request() {
        if (hfun && eval_count % MY_NLOPT_EVAL_REFRESH == 0) {
            const float xf = float(eval_count) / float(max_eval);
            int stop_request = (*hfun)(hdata, xf, nullptr);
            return (stop_request != 0);
        }
        return false;
    }

    void start_message(const char *msg) {
        if (hfun) {
            (*hfun)(hdata, 0.0, msg);
        }
    }

    void eval_f(const gsl_vector *x) {
        fit->run->mffun.f(x, (void *) fit, f);
    }

    void eval_df(const gsl_vector *x) {
        fit->run->mffun.df(x, (void *) fit, J);
    }

    int n() const { return fit->run->mffun.n; }
};

static double
objective_func(unsigned n, const double *x, double *grad, void *_data)
{
    objective_data *data = (objective_data *) _data;
    if (data->check_for_stop_request()) {
        nlopt_force_stop(data->opt);
    }
    gsl_vector_const_view xv = gsl_vector_const_view_array(x, n);
    data->eval_f(&xv.vector);
    const double chisq = pow2(gsl_blas_dnrm2(data->f));
    if (grad) {
        data->eval_df(&xv.vector);
        for (int k = 0; k < (int) n; k++) { // Iterate over the fit parameters.
            double der = 0.0;
            for (int i = 0; i < data->n(); i++) { // Iterate over the points.
                der += 2 * gsl_vector_get(data->f, i) * gsl_matrix_get(data->J, i, k);
            }
            grad[k] = der;
        }
    }
    data->eval_count ++;
    return chisq;
}

static void
set_initial_seeds(fit_engine *fit, seeds *seeds, gsl_vector *x)
{
    const int dim = fit->parameters->number;
    for(int j = 0; j < dim; j++) {
        const double xc = fit_engine_get_seed_value(fit, &fit->parameters->values[j], &seeds->values[j]);
        gsl_vector_set(x, j, xc);
    }
}

static void
set_optimizer_bounds(nlopt_opt opt, fit_engine *fit, seeds *seeds, const gsl_vector *x, int limits_type)
{
    const int dim = fit->parameters->number;
    double_array8 lower_bounds(dim), upper_bounds(dim);
    for(int j = 0; j < dim; j++) {
        const double xc = gsl_vector_get(x, j);
        if(seeds->values[j].type == SEED_RANGE) {
            const double delta = seeds->values[j].delta;
            lower_bounds[j] = xc - delta;
            upper_bounds[j] = xc + delta;
        } else {
            if (limits_type == OPT_BOUNDS_RESTRICT) {
                const fit_param_t fp = fit->parameters->values[j];
                const double delta = fit_engine_estimate_param_grid_step(fit, x, &fp, fabs(xc) > 0.1 ? fabs(xc) / 2 : 0.05);
                lower_bounds[j] = xc - delta;
                upper_bounds[j] = xc + delta;
            } else {
                lower_bounds[j] = -HUGE_VAL;
                upper_bounds[j] = +HUGE_VAL;
            }
        }
        fprintf(stderr, "parameter: %d, lower: %g upper: %g bound\n", j, lower_bounds[j], upper_bounds[j]);
    }
    nlopt_set_lower_bounds(opt, lower_bounds);
    nlopt_set_upper_bounds(opt, upper_bounds);
}

static void report_global_search_outcome(int nlopt_status, gsl::vector& x, double chisq, str_ptr analysis) {
    if (nlopt_status < 0) {
        str_printf(analysis, "Global search failed with error code: %d\n", nlopt_status);
        return;
    }

    str_printf(analysis, "Found global minimum");
    for (int i = 0; i < x.size(); i++) {
        str_printf_add(analysis, " %g", x[i]);
    }
    str_printf_add(analysis, " with chi square: %g\n", chisq);
    str_printf_add(analysis, "NLopt termination code: %d\n", nlopt_status);
}

// Perform a global optimization search and return the results in the vector "x". Return a
// status code indicating success or failure. The user interface is updated.
static int
global_search_nlopt(fit_engine *fit, seeds *seeds, str_ptr analysis, gui_hook_func_t hfun, void *hdata, gsl::vector& x) {
    const int dim = fit->parameters->number;
    // Prepare the NLOpt optimizer.
    nlopt_opt opt = nlopt_create(NLOPT_GN_CRS2_LM, dim);
    objective_data data(fit, opt, hfun, hdata, MY_NLOPT_MAXEVAL);

    // Send message to the UI.
    data.start_message("Running NLopt search...");

    set_initial_seeds(fit, seeds, x);
    set_optimizer_bounds(opt, fit, seeds, x, OPT_BOUNDS_RESTRICT);
    nlopt_set_min_objective(opt, objective_func, (void *) &data);
    nlopt_set_maxeval(opt, MY_NLOPT_MAXEVAL);
    nlopt_set_stopval(opt, 1e-4);
    nlopt_set_ftol_rel(opt, 1e-4);

    double chisq;
    // Perform the actual NLopt optimization.
    int nlopt_status = nlopt_optimize(opt, x.data(), &chisq);

    // Report global seach information in text format.
    report_global_search_outcome(nlopt_status, x, chisq, analysis);
    nlopt_destroy(opt);

    return (nlopt_status >= 0 ? GSEARCH_SUCCESS : (nlopt_status == NLOPT_FORCED_STOP ? GSEARCH_STOP_REQUEST : GSEARCH_FAILURE));
}

void
nlopt_fit(fit_engine *parent_fit, spectrum *spectrum, gsl::vector& x, seeds *seeds, lmfit_result *result, str_ptr analysis, int preserve_init_stack,
          gui_hook_func_t hfun, void *hdata)
{
    // Create a copy of the original fit engine.
    fit_engine *fit = fit_engine_clone(parent_fit);

    // Prepare the new fit engine using the spectrum and enable subsumpling.
    fit_engine_prepare(fit, spectrum, FIT_RESET_ACQUISITION|FIT_ENABLE_SUBSAMPLING);

    int gsearch_status = global_search_nlopt(fit, seeds, analysis, hfun, hdata, x);
    fit_engine_disable(fit);
    fit_engine_free(fit);

    if(gsearch_status == GSEARCH_SUCCESS) {
        int stop_request;
        fit_engine_prepare(parent_fit, spectrum, FIT_RESET_ACQUISITION);
        fit_engine_lmfit(parent_fit, x, result, parent_fit->config, hfun, hdata, stop_request);
        result->nb_points = spectra_points(parent_fit->run->spectr);
        if (!preserve_init_stack) {
            /* we take care to commit the results obtained from the fit */
            fit_engine_commit_fit_results(parent_fit, x);
        }
        fit_engine_disable(parent_fit);
    }
}
