#include <nlopt.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>

#include "math-utils.h"
#include "nlopt_fit.h"
#include "fit_result.h"
#include "cmpl.h"

// To be used as the user data in the nlopt ojective function.
// Hold the resources to evaluate the f and df using the fit_engine's
// multifit fdf function.
struct objective_data {
    fit_engine *fit;
    gsl_vector *f;
    gsl_matrix *J;

    objective_data(fit_engine *_fit): fit(_fit) {
        f = gsl_vector_alloc(fit->run->mffun.n);
        J = gsl_matrix_alloc(fit->run->mffun.n, fit->run->mffun.p);
    }

    ~objective_data() {
        gsl_vector_free(f);
        gsl_matrix_free(J);
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

    return chisq;
}

static void
set_optimizer_bounds(nlopt_opt opt, fit_engine *fit, seeds *seeds, double_array x)
{
    const int dim = fit->parameters->number;
    double_array8 lower_bounds(dim), upper_bounds(dim);
    for(int j = 0; j < dim; j++) {
        x[j] = fit_engine_get_seed_value(fit, &fit->parameters->values[j], &seeds->values[j]);
        if(seeds->values[j].type == SEED_RANGE) {
            const double delta = seeds->values[j].delta;
            lower_bounds[j] = x[j] - delta;
            upper_bounds[j] = x[j] + delta;
        } else {
            lower_bounds[j] = -HUGE_VAL;
            upper_bounds[j] = +HUGE_VAL;
        }
    }
    nlopt_set_lower_bounds(opt, lower_bounds.data());
    nlopt_set_upper_bounds(opt, upper_bounds.data());
}

int
nlopt_fit(fit_engine *fit, seeds *seeds, lmfit_result *result, str_ptr analysis, int preserve_init_stack,
          gui_hook_func_t hfun, void *hdata)
{
    const int dim = fit->parameters->number;
    fit_result nlopt_result[1];

    if(hfun) {
        (*hfun)(hdata, 0.0, "Running NLopt search...");
    }

    objective_data data(fit);
    fit_result_init(nlopt_result, fit);
    nlopt_opt opt = nlopt_create(NLOPT_LD_MMA, dim);
    double_array8 x(dim);
    set_optimizer_bounds(opt, fit, seeds, x);
    nlopt_set_min_objective(opt, objective_func, (void *) &data);
    nlopt_set_xtol_rel(opt, 1e-4);

    fit_engine_prepare(fit, fit->run->spectr, FIT_KEEP_ACQUISITION|FIT_ENABLE_SUBSAMPLING);

    fprintf(stderr, "initial point:");
    for (int i = 0; i < dim; i++) {
        fprintf(stderr, " %g", x[i]);
    }
    const double minf0 = objective_func(dim, x.data(), nullptr, &data);
    fprintf(stderr, "\nwith objective function: %g\n", minf0);

    double minf;
    if (nlopt_optimize(opt, x.data(), &minf) < 0) {
        // TO BE done properly.
        fprintf(stderr, "nlopt failed!\n");
        return 1;
    }
    fprintf(stderr, "found minimum at:");
    for (int i = 0; i < dim; i++) {
        fprintf(stderr, " %g", x[i]);
    }
    fprintf(stderr, "\nwith objective function: %g\n", minf);
    return 0;
}
