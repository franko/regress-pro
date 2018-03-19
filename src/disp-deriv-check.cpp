#include <gsl/gsl_deriv.h>

#include "disp-deriv-check.h"
#include "pod_vector.h"

struct disp_deriv_data {
    disp_t *d;
    double *param_ptr;
    double wavelength;
};

static double n_eval_real(double x, void *params)
{
    disp_deriv_data *data = (disp_deriv_data *) params;
    *data->param_ptr = x;
    cmpl n = n_value(data->d, data->wavelength);
    return std::real(n);
}

static double n_eval_imag(double x, void *params)
{
    disp_deriv_data *data = (disp_deriv_data *) params;
    *data->param_ptr = x;
    cmpl n = n_value(data->d, data->wavelength);
    return std::imag(n);
}

bool disp_deriv_check(disp_t *d, double wavelength) {
    const int n_parameters = disp_get_number_of_params(d);
    disp_deriv_data data[1] = {{d, nullptr, wavelength}};

    pod::array<cmpl> num_deriv(n_parameters);
    pod::array<cmpl> num_abserr(n_parameters);

    gsl_function F;
    F.params = (void *) data;
    for (int i = 0; i < n_parameters; i++) {
        data->param_ptr = disp_map_param(d, i);

        double rderiv, ideriv;

        F.function = &n_eval_real;
        double rabserr, iabserr;
        const double p0 = *data->param_ptr;
        const double h = std::max(1e-6, p0 * 1e-4);
        gsl_deriv_central(&F, p0, h, &rderiv, &rabserr);
        *data->param_ptr = p0;

        F.function = &n_eval_imag;
        gsl_deriv_central(&F, p0, h, &ideriv, &iabserr);
        *data->param_ptr = p0;

        num_deriv[i] = cmpl(rderiv, ideriv);
        num_abserr[i] = cmpl(rabserr, iabserr);
    }

    pod::array<cmpl> computed_deriv(n_parameters);
    n_value_deriv(d, &computed_deriv, wavelength);

    bool success = true;
    for (int i = 0; i < n_parameters; i++) {
        fprintf(stderr, "Parameter %d\n", i);
        cmpl n = num_deriv[i], c = computed_deriv[i], e = num_abserr[i];
        if (std::real(n - c) > std::real(e) || std::imag(n - c) > std::imag(e)) {
            fprintf(stderr, "FAIL REAL numeric: %g +/- %g, computed: %g\n", std::real(n), std::real(e), std::real(c));
            fprintf(stderr, "FAIL IMAG numeric: %g +/- %g, computed: %g\n", std::imag(n), std::imag(e), std::imag(c));
            success = false;
        } else {
            fprintf(stderr, "PASS REAL numeric: %g +/- %g, computed: %g\n", std::real(n), std::real(e), std::real(c));
            fprintf(stderr, "PASS IMAG numeric: %g +/- %g, computed: %g\n", std::imag(n), std::imag(e), std::imag(c));
        }
    }
    return success;
}
