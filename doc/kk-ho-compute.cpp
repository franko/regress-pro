#include <cstdio>
#include <gsl/gsl_integration.h>

#include "libcanvas.h"

using namespace libcanvas;

template <typename Float> Float sqr(Float x) { return x * x; }

struct HOModel {
    double en, eg, a, phi;
    double e;
};

double hoImaginaryPart(const HOModel& ho, double e) {
    double dr = sqr(ho.en) - sqr(e), di = ho.eg * e;
    double c = std::cos(ho.phi), s = std::sin(ho.phi);
    return ho.a * (-c * di + s * dr) / (sqr(dr) + sqr(di));
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
    Plot plot(Plot::ShowUnits | Plot::AutoLimits);
    plot.SetClipMode(false);

    const int integLimit = 256;
    gsl_integration_workspace *gs = gsl_integration_workspace_alloc(integLimit);
    const double eCut = 12.0;

    HOModel hoTest{3.5, 0.3, 10.0, math::Pi() / 4, 0.0};

    gsl_function hoImgPartF;
    hoImgPartF.function = &hoImgPartFfun;
    hoImgPartF.params = &hoTest;

    gsl_function hoKKImgPartF;
    hoKKImgPartF.function = &hoKKImgPartFfun;
    hoKKImgPartF.params = &hoTest;

    double epsabs = 1e-5, epsrel = 1e-5;

    Path lineN{}, lineNRef{}, lineK;

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

        if (i == 0) {
            lineN.MoveTo(eEval, result1 + result2);
        } else {
            lineN.LineTo(eEval, result1 + result2);
        }

        if (i == 0) {
            lineNRef.MoveTo(eEval, -hoRealPart(hoTest, hoTest.e));
        } else {
            lineNRef.LineTo(eEval, -hoRealPart(hoTest, hoTest.e));
        }

        if (i == 0) {
            lineK.MoveTo(eEval, -hoImaginaryPart(hoTest, hoTest.e));
        } else {
            lineK.LineTo(eEval, -hoImaginaryPart(hoTest, hoTest.e));
        }
    }

    plot.AddStroke(lineNRef, color::Gray, 1.0);
    plot.AddStroke(lineN, color::Blue, 1.5);
    plot.AddStroke(lineK, color::Red,  1.5);

    Window window;
    window.Attach(plot, "");
    window.Start(640, 480, WindowResize);
    window.Wait();
    return 0;
}
