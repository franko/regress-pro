#include <cstdio>
#include <complex>
#include <gsl/gsl_integration.h>

#include "elem/elem.h"

using namespace elem;

template <typename Float> Float sqr(Float x) { return x * x; }

template <typename Float>
static Float EnergyToWavelength(const Float energy) {
    return 1240.0 / energy;
}

struct HOFirstOrderDerivModel {
    double en, eg, a;
    double e;
    bool clamp_k_to_zero;
};

double hoImpImaginaryPart(const HOFirstOrderDerivModel& ho, double e) {
    const std::complex<double> ld{sqr(ho.en) - sqr(e), ho.eg * e};
    const double normalize_factor = 1 / (4 * ho.en);
    const double k = normalize_factor * (4 * ho.eg * e * ho.en * std::real(ld)) / sqr(std::norm(ld));
    return (ho.clamp_k_to_zero && k >= 0.0) ? 0.0 : k;
}

double hoImpRealPart(const HOFirstOrderDerivModel& ho, double e) {
    const std::complex<double> ld{sqr(ho.en) - sqr(e), ho.eg * e};
    const double normalize_factor = 1 / (4 * ho.en);
    const double n = normalize_factor * (
        (2 * ho.en) / std::norm(ld)
        - (4 * ho.en * sqr(std::real(ld))) / sqr(std::norm(ld))
    );
    return n;
}

static double hoImpKKImgIntegrandF(double x, void *param) {
    const HOFirstOrderDerivModel *ho = (const HOFirstOrderDerivModel *) param;
    double chi2 = hoImpImaginaryPart(*ho, x);
    return (2 / math::Pi()) * x * chi2 / (sqr(x) - sqr(ho->e));
}

static double hoImpKKImgPValIntegrandF(double x, void *param) {
    const HOFirstOrderDerivModel *ho = (const HOFirstOrderDerivModel *) param;
    double chi2 = hoImpImaginaryPart(*ho, x);
    return (2 / math::Pi()) * x * chi2 / (x + ho->e);
}

int main() {
    InitializeFonts();

    const int integLimit = 1024;
    gsl_integration_workspace *gs = gsl_integration_workspace_alloc(integLimit);
    const double eCut = 12.0;

    HOFirstOrderDerivModel hoTest{3.5, 0.3, 10.0, false};

    gsl_function kKPValIntegrandF;
    kKPValIntegrandF.function = &hoImpKKImgPValIntegrandF;
    kKPValIntegrandF.params = &hoTest;

    gsl_function kKIntegrandF;
    kKIntegrandF.function = &hoImpKKImgIntegrandF;
    kKIntegrandF.params = &hoTest;

    double epsabs = 1e-6, epsrel = 1e-10;

    Path lineN{}, lineNRef{}, lineK;
    Path lineNLam{}, lineNRefLam{}, lineKLam;

    const double lambda_min = 300.0, lambda_max = 800.0;

    const double e1 = 0.01, e2 = 10.0;
    const int nIntervals = 1024;
    for (int i = 0; i <= nIntervals; i++) {
        const double eEval = e1 + (e2 - e1) * i / nIntervals;

        hoTest.e = eEval;

        double result1, abserr1;
        gsl_integration_qawc(&kKPValIntegrandF, 0.0, eCut, eEval, epsabs, epsrel, integLimit, gs, &result1, &abserr1);

        double result2, abserr2;
        gsl_integration_qagiu(&kKIntegrandF, eCut, epsabs, epsrel, integLimit, gs, &result2, &abserr2);

        printf("%g, %g, %g\n", eEval, result1 + result2, hoImpRealPart(hoTest, hoTest.e));

        lineN.LineTo(eEval, result1 + result2);
        lineNRef.LineTo(eEval, -hoImpRealPart(hoTest, hoTest.e));
        lineK.LineTo(eEval, -hoImpImaginaryPart(hoTest, hoTest.e));

        const double lambda_eval = EnergyToWavelength(eEval);
        if (lambda_eval >= lambda_min && lambda_eval <= lambda_max) {
            lineNLam.LineTo(lambda_eval, result1 + result2);
            lineNRefLam.LineTo(lambda_eval, -hoImpRealPart(hoTest, hoTest.e));
            lineKLam.LineTo(lambda_eval, -hoImpImaginaryPart(hoTest, hoTest.e));
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
