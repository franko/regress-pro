#include <cstdio>
#include <complex>
#include <cmath>
#include <gsl/gsl_integration.h>

#include "elem/elem.h"

using namespace elem;

typedef std::complex<double> cmpl;

struct kramers_params {
    double a;
    double en;
    double eg;
    double phi;
};

struct disp_kramers {
    int n;
    kramers_params oscillators[2]; // Hard coded number of oscillators.
};

struct KramersKKData {
    disp_kramers kramers[1];
    double e;
};

/* Generated with sympy using the command: kramers-sympy.py
 * ../src/kramers-template.cpp */
static cmpl kramers_epsilon_raw(const disp_kramers *kramers, double e) {
    double er_sum = 1.0, ei_sum = 0.0;
    for (int k = 0; k < kramers->n; k++) {
        const kramers_params *k_oscillator = &kramers->oscillators[k];
        const double a = k_oscillator->a, en = k_oscillator->en,
                     eg = k_oscillator->eg, phi = k_oscillator->phi;

        /* Here we provided the variables a, en, eg, phi and e. These variables
           are used by the sympy automatic code. */

        const auto x0 = std::pow(e, 2);
        const auto x1 = std::pow(en, 2);
        const auto x2 = x0 - x1;
        const auto x3 = std::pow(x2, 2);
        const auto x4 = (x0 + x1) * std::sin(phi) / (x0 * x1 + x3);
        const auto x5 = 2 * std::cos(phi);
        const auto x6 = x2 * x4;
        const auto x7 = (1.0 / 2.0) * a / (std::pow(eg, 2) * x0 + x3);

        double eps1 = -x7 * (eg * en * x0 * x4 + x2 * (x5 - x6));
        double eps2 = -e * x7 * (eg * (-x5 + x6) + en * x6);
        er_sum += eps1;
        ei_sum += eps2;
    }
    return cmpl{er_sum, -ei_sum};
}

template <typename Float>
static Float EnergyToWavelength(const Float energy) {
    return 1240.0 / energy;
}

template <typename Float> Float sqr(Float x) { return x * x; }

static double KKIntegrandF(double x, void *param) {
    const KramersKKData *data = (const KramersKKData *) param;
    const cmpl epsilon = kramers_epsilon_raw(data->kramers, x);
    const double chi2 = -epsilon.imag();
    return (2 / math::Pi()) * x * chi2 / (sqr(x) - sqr(data->e));
}

static double KKPValIntegrandF(double x, void *param) {
    const KramersKKData *data = (const KramersKKData *) param;
    const cmpl epsilon = kramers_epsilon_raw(data->kramers, x);
    const double chi2 = -epsilon.imag();
    return (2 / math::Pi()) * x * chi2 / (x + data->e);
}

int main() {
    InitializeFonts();

    const int integ_limit = 1024;
    gsl_integration_workspace *gs = gsl_integration_workspace_alloc(integ_limit);
    const double e_cut = 12.0;

    KramersKKData data;
    data.kramers->n = 1;
    data.kramers->oscillators[0].a   = 10.0;
    data.kramers->oscillators[0].en  = 3.5;
    data.kramers->oscillators[0].eg  = 0.5;
    data.kramers->oscillators[0].phi = 0.0;

    gsl_function kk_pval_integrand;
    kk_pval_integrand.function = &KKPValIntegrandF;
    kk_pval_integrand.params = &data;

    gsl_function kk_integrand;
    kk_integrand.function = &KKIntegrandF;
    kk_integrand.params = &data;

    double epsabs = 1e-6, epsrel = 1e-7;

    Path lineN{}, lineNRef{}, lineK;
    Path lineNLam{}, lineNRefLam{}, lineKLam;

    const double lambda_min = 300.0, lambda_max = 800.0;

    const double e1 = 0.01, e2 = 10.0;
    const int n_intervals = 1024;
    for (int i = 0; i <= n_intervals; i++) {
        const double e_eval = e1 + (e2 - e1) * i / n_intervals;
        data.e = e_eval;

        double result_pval, abserr_pval;
        gsl_integration_qawc(&kk_pval_integrand, 0.0, e_cut, e_eval, epsabs, epsrel, integ_limit, gs, &result_pval, &abserr_pval);

        double result_inf, abserr_inf;
        gsl_integration_qagiu(&kk_integrand, e_cut, epsabs, epsrel, integ_limit, gs, &result_inf, &abserr_inf);

        result_inf = 1.0;

        const cmpl eps_expected = kramers_epsilon_raw(data.kramers, data.e);
        printf("%g, %g, %g\n", e_eval, result_pval + result_inf, eps_expected.real());

        lineN.LineTo(e_eval, result_pval + result_inf);
        lineNRef.LineTo(e_eval, eps_expected.real());
        lineK.LineTo(e_eval, -eps_expected.imag());

        const double lambda_eval = EnergyToWavelength(e_eval);
        if (lambda_eval >= lambda_min && lambda_eval <= lambda_max) {
            lineNLam.LineTo(lambda_eval, result_pval + result_inf);
            lineNRefLam.LineTo(lambda_eval, eps_expected.real());
            lineKLam.LineTo(lambda_eval, -eps_expected.imag());
        }
    }

    Plot plot(Plot::ShowUnits | Plot::AutoLimits);
    plot.SetTitle("N&K vs energy");
    plot.SetXAxisTitle("Energy (eV)");
    plot.AddStroke(lineNRef, color::Gray, 1.0);
    plot.AddStroke(lineN, color::Blue, 1.5);
    plot.AddStroke(lineK, color::Red,  1.5);

    Plot plot_lam(Plot::ShowUnits | Plot::AutoLimits);
    plot_lam.SetTitle("N&K vs wavelength");
    plot_lam.SetXAxisTitle("Wavelength (nm)");
    plot_lam.AddStroke(lineNRefLam, color::Gray, 1.0);
    plot_lam.AddStroke(lineNLam, color::Blue, 1.5);
    plot_lam.AddStroke(lineKLam, color::Red,  1.5);

    Window window("v..");
    window.Attach(plot, "1");
    window.Attach(plot_lam, "2");
    window.Start(640, 920, WindowResize);
    window.Wait();
    return 0;
}
