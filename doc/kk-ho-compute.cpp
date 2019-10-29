#include <cstdio>
#include <cmath>
#include <gsl/gsl_integration.h>

#include "libcanvas.h"

using namespace libcanvas;

template <typename Float> Float sqr(Float x) { return x * x; }

template <typename Float>
static Float EnergyToWavelength(const Float energy) {
    return 1240.0 / energy;
}

struct HOModel {
    double en, eg, a, phi;
    double e;
    bool clamp_k_to_zero;
};

double hoImaginaryPart(const HOModel& ho, double e) {
    const double dr = sqr(ho.en) - sqr(e), di = ho.eg * e;
    const double c = std::cos(ho.phi), s = std::sin(ho.phi);
    const double k = ho.a * (-c * di + s * dr) / (sqr(dr) + sqr(di));
    return (ho.clamp_k_to_zero && k >= 0.0) ? 0.0 : k;
}

double hoRealPart(const HOModel& ho, double e) {
    double dr = sqr(ho.en) - sqr(e), di = ho.eg * e;
    double c = std::cos(ho.phi), s = std::sin(ho.phi);
    return ho.a * (c * dr + s * di) / (sqr(dr) + sqr(di));
}

static double hoKKImgPartFfun(double x, void *param) {
    const HOModel *ho = (const HOModel *) param;
    double chi2 = hoImaginaryPart(*ho, x);
    return (2 / math::Pi()) * x * chi2 / (sqr(x) - sqr(ho->e));
}

static double hoImgPartFfun(double x, void *param) {
    const HOModel *ho = (const HOModel *) param;
    double chi2 = hoImaginaryPart(*ho, x);
    return (2 / math::Pi()) * x * chi2 / (x + ho->e);
}

int main() {
    InitializeFonts();

    const int integLimit = 256;
    gsl_integration_workspace *gs = gsl_integration_workspace_alloc(integLimit);
    const double eCut = 12.0;

    HOModel hoTest{3.5, 0.3, 10.0, math::Pi() / 4, 0.0, true};

    gsl_function hoImgPartF;
    hoImgPartF.function = &hoImgPartFfun;
    hoImgPartF.params = &hoTest;

    gsl_function hoKKImgPartF;
    hoKKImgPartF.function = &hoKKImgPartFfun;
    hoKKImgPartF.params = &hoTest;

    double epsabs = 1e-5, epsrel = 1e-5;

    Path lineN{}, lineNRef{}, lineK;
    Path lineNLam{}, lineNRefLam{}, lineKLam;

    const double lambda_min = 300.0, lambda_max = 800.0;

    const double e1 = 0.01, e2 = 10.0;
    const int nIntervals = 1024;
    for (int i = 0; i <= nIntervals; i++) {
        const double eEval = e1 + (e2 - e1) * i / nIntervals;

        hoTest.e = eEval;

        double result1, abserr1;
        gsl_integration_qawc(&hoImgPartF, 0.0, eCut, eEval, epsabs, epsrel, integLimit, gs, &result1, &abserr1);

        double result2, abserr2;
        gsl_integration_qagiu(&hoKKImgPartF, eCut, epsabs, epsrel, integLimit, gs, &result2, &abserr2);

        printf("%g, %g, %g\n", eEval, result1 + result2, hoRealPart(hoTest, hoTest.e));

        lineN.LineTo(eEval, result1 + result2);
        lineNRef.LineTo(eEval, -hoRealPart(hoTest, hoTest.e));
        lineK.LineTo(eEval, -hoImaginaryPart(hoTest, hoTest.e));

        const double lambda_eval = EnergyToWavelength(eEval);
        if (lambda_eval >= lambda_min && lambda_eval <= lambda_max) {
            lineNLam.LineTo(lambda_eval, result1 + result2);
            lineNRefLam.LineTo(lambda_eval, -hoRealPart(hoTest, hoTest.e));
            lineKLam.LineTo(lambda_eval, -hoImaginaryPart(hoTest, hoTest.e));
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
