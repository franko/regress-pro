#include <cstdio>
#include <gsl/gsl_integration.h>

constexpr double pi() { return 3.14159265358979323846; }

template <typename Float> Float sqr(Float x) { return x * x; }

struct HOModel {
    double en, eg, a; // phi;
    double e;
};

double hoImaginaryPart(const HOModel& ho, double e) {
    return - (ho.a * ho.eg * e) / (sqr(sqr(ho.en) - sqr(e)) + sqr(ho.eg) * sqr(e));
}

double hoRealPart(const HOModel& ho, double e) {
    return (ho.a * (sqr(ho.en) - sqr(e))) / (sqr(sqr(ho.en) - sqr(e)) + sqr(ho.eg) * sqr(e));
}

static double hoKKImgPartFfun(double x, void *param) {
    const HOModel *ho = (const HOModel *) param;
    double chi2 = hoImaginaryPart(*ho, x);
    return (2 / pi()) * x * chi2 / (sqr(x) - sqr(ho->e));
}

static double hoImgPartFfun(double x, void *param) {
    const HOModel *ho = (const HOModel *) param;
    double chi2 = hoImaginaryPart(*ho, x);
    return (2 / pi()) * x * chi2 / (x + ho->e);
}

int main() {
    const int intLimit = 128;
    gsl_integration_workspace *gs = gsl_integration_workspace_alloc(intLimit);
    const double eCut = 12.0;

    HOModel hoTest{3.5, 0.9, 10.0, 0.0};
    hoTest.e = 0.0;

    gsl_function hoImgPartF;
    hoImgPartF.function = &hoImgPartFfun;
    hoImgPartF.params = &hoTest;

    gsl_function hoKKImgPartF;
    hoKKImgPartF.function = &hoKKImgPartFfun;
    hoKKImgPartF.params = &hoTest;

    double epsabs = 1e-5, epsrel = 1e-5;

    const double e1 = 0.5, e2 = 8.0;
    for (int i = 0; i <= 128; i++) {
        const double eEval = e1 + (e2 - e1) * i / 128;

        hoTest.e = eEval;

        double result1, abserr1;
        gsl_integration_qawc(&hoImgPartF, 0.0, eCut, eEval, epsabs, epsrel, intLimit, gs, &result1, &abserr1);

        double result2, abserr2;
        gsl_integration_qagiu(&hoKKImgPartF, eCut, epsabs, epsrel, intLimit, gs, &result2, &abserr2);

        printf("%g, %g\n", result1 + result2, hoRealPart(hoTest, hoTest.e));
    }

    return 0;
}
