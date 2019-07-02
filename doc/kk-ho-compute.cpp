#include "window.h"
#include "debug_log.h"
#include "path.h"

#include <cstdio>
#include <gsl/gsl_integration.h>

constexpr double m_pi() { return 3.14159265358979323846; }

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
    return (2 / m_pi()) * x * chi2 / (sqr(x) - sqr(ho->e));
}

static double hoImgPartFfun(double x, void *param) {
    const HOModel *ho = (const HOModel *) param;
    double chi2 = hoImaginaryPart(*ho, x);
    return (2 / m_pi()) * x * chi2 / (x + ho->e);
}

int main() {
    graphics::initialize_fonts();
    graphics::window win;
    graphics::plot p(graphics::plot::show_units | graphics::plot::auto_limits);
    p.set_clip_mode(false);

    const int integLimit = 256;
    gsl_integration_workspace *gs = gsl_integration_workspace_alloc(integLimit);
    const double eCut = 12.0;

    HOModel hoTest{3.5, 0.3, 10.0, m_pi() / 4, 0.0};

    gsl_function hoImgPartF;
    hoImgPartF.function = &hoImgPartFfun;
    hoImgPartF.params = &hoTest;

    gsl_function hoKKImgPartF;
    hoKKImgPartF.function = &hoKKImgPartFfun;
    hoKKImgPartF.params = &hoTest;

    double epsabs = 1e-5, epsrel = 1e-5;

    auto lineN = new graphics::path(), lineNRef = new graphics::path();
    auto lineK = new graphics::path();

    const double e1 = 0.01, e2 = 10.0;
    const int nIntervals = 256;
    for (int i = 0; i <= 256; i++) {
        const double eEval = e1 + (e2 - e1) * i / nIntervals;

        hoTest.e = eEval;

        double result1, abserr1;
        gsl_integration_qawc(&hoImgPartF, 0.0, eCut, eEval, epsabs, epsrel, integLimit, gs, &result1, &abserr1);

        double result2, abserr2;
        gsl_integration_qagiu(&hoKKImgPartF, eCut, epsabs, epsrel, integLimit, gs, &result2, &abserr2);

        printf("%g, %g, %g\n", eEval, result1 + result2, hoRealPart(hoTest, hoTest.e));

        if (i == 0) {
            lineN->move_to(eEval, result1 + result2);
        } else {
            lineN->line_to(eEval, result1 + result2);
        }

        if (i == 0) {
            lineNRef->move_to(eEval, -hoRealPart(hoTest, hoTest.e));
        } else {
            lineNRef->line_to(eEval, -hoRealPart(hoTest, hoTest.e));
        }

        if (i == 0) {
            lineK->move_to(eEval, -hoImaginaryPart(hoTest, hoTest.e));
        } else {
            lineK->line_to(eEval, -hoImaginaryPart(hoTest, hoTest.e));
        }
    }

    agg::rgba8 none(0,0,0,0);
    agg::rgba8 blue(0, 0, 180, 255);
    agg::rgba8 red(180, 0, 0, 255);
    agg::rgba8 gray(50, 50, 50, 255);
    p.add(lineNRef, gray, 1.0, none, graphics::property::stroke);
    p.add(lineN, blue, 1.5, none, graphics::property::stroke);
    p.add(lineK, red,  1.5, none, graphics::property::stroke);
    int index = win.attach(&p, "");
    win.start(640, 480, graphics::window_resize);
    win.slot_refresh(index);
    p.commit_pending_draw();
    win.wait();

    return 0;
}
