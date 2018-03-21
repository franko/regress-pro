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

    pod::array<cmpl> computed_deriv(n_parameters);
    n_value_deriv(d, &computed_deriv, wavelength);

    bool deriv_pass = true;
    gsl_function F;
    F.params = (void *) data;
    for (int i = 0; i < n_parameters; i++) {
        const cmpl cder = computed_deriv[i];
        data->param_ptr = disp_map_param(d, i);

        double nder[3], abserr[3];

        F.function = &n_eval_real;
        const double p0 = *data->param_ptr;
        const double h = std::max(1e-6, p0 * 1e-4);
        gsl_deriv_central(&F, p0, h, &nder[0], &abserr[0]);
        gsl_deriv_forward(&F, p0, h, &nder[1], &abserr[1]);
        gsl_deriv_backward(&F, p0, h, &nder[2], &abserr[2]);
        *data->param_ptr = p0;

        bool deriv_pass_real = false;
        for (int q = 0; q < 3; q++) {
            const bool qpass = (fabs(nder[q] - std::real(cder)) <= abserr[q]);
            deriv_pass_real = deriv_pass_real || qpass;
        }
        if (!deriv_pass_real) {
            fprintf(stderr, "FAIL parameter %d REAL (%g nm) numeric: (%g +/- %g) %g +/- %g (%g +/- %g), computed: %g\n", i, wavelength, nder[1], abserr[1], nder[0], abserr[0], nder[2], abserr[2], std::real(cder));
        }

        F.function = &n_eval_imag;
        gsl_deriv_central(&F, p0, h, &nder[0], &abserr[0]);
        gsl_deriv_forward(&F, p0, h, &nder[1], &abserr[1]);
        gsl_deriv_backward(&F, p0, h, &nder[2], &abserr[2]);
        *data->param_ptr = p0;

        bool deriv_pass_imag = false;
        for (int q = 0; q < 3; q++) {
            const bool qpass = (fabs(nder[q] - std::imag(cder)) <= abserr[q]);
            deriv_pass_imag = deriv_pass_imag || qpass;
        }
        if (!deriv_pass_imag) {
            fprintf(stderr, "FAIL parameter %d IMAG (%g nm) numeric: (%g +/- %g) %g +/- %g (%g +/- %g), computed: %g\n", i, wavelength, nder[1], abserr[1], nder[0], abserr[0], nder[2], abserr[2], std::imag(cder));
        }
        deriv_pass = deriv_pass && deriv_pass_real && deriv_pass_imag;
    }

    return deriv_pass;
}
