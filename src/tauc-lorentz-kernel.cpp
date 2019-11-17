cmpl
tauc_lorentz_n_value(const disp_t *d, double lambda)
{
    const struct disp_fb *fb = &d->disp.fb;
    const int nb = fb->n;
    const double en = TL_EV_NM / lambda;
    const double eg_delta = 1.0e-4;

    /* If Eg is negative use zero instead. A negative Eg is not meaningful. */
    const double eg = (fb->eg >= 0 ? fb->eg : 0.0);
    double er_sum = fb->n_inf, ei_sum = 0.0;
    for (int k = 0; k < nb; k++) {
        double a, e0, c;
        oscillator_parameters(fb->form, fb->osc + k, a, e0, c);

const auto x0 = std::pow(en, 2);
const auto x1 = std::pow(c, 2);
const auto x2 = std::pow(eg, 2);
const auto x3 = x1*x2;
const auto x4 = std::pow(e0, 2);
const auto x5 = std::sqrt(x3 + std::pow(-x2 + x4, 2));
const auto x6 = -x1;
const auto x7 = 4*x4 + x6;
const auto x8 = -x4;
const auto x9 = x0 + x8;
const auto x10 = (1.0/2.0)*x1 + x9;
const auto x11 = a/(M_PI*((1.0/4.0)*x1*x7 + std::pow(x10, 2)));
const auto x12 = c*e0;
const auto x13 = x11*x12;
const auto x14 = std::log(eg + en);
const auto x15 = -eg + en;
const auto x16 = eg < en;
const auto x17 = ((x16) ? (
   std::log(x15)
)
: (
   std::log(eg - en)
));
const auto x18 = x12/en;
const auto x19 = 2*eg;
const auto x20 = x13*x19;
const auto x21 = std::sqrt(x7);
const auto x22 = 1.0/x21;
const auto x23 = 1.0/c;
const auto x24 = x2 + x4;
const auto x25 = x11/e0;
const auto x26 = x25*(x24*x9 + x3);
const auto x27 = eg*x21;
const auto x28 = (1.0/2.0)*c;
const auto x29 = x25*(x0*(x2 + x8) + x3 - x4*(3*x2 + x4));
const auto x30 = 1.0/x24;

        double eps1 = ((en > eg - eg_delta && en < eg + eg_delta) ? (
   2*en*x13*std::log(4*x0/x5)
)
: (
   -x11*x18*(x0 + x2)*(-x14 + x17) + x20*(x14 + x17 - std::log(x5))
)) + ((c < 1.9999999999*e0) ? (
   e0*x10*x11*x19*x22*(2*std::atan(x22*x23*(-2*x2 + 2*x4 + x6)) + M_PI) + x22*x28*x29*std::log((x24 + x27)/(x24 - x27)) - x26*(std::atan(x23*(-x19 + x21)) - std::atan(x23*(x19 + x21)) + M_PI)
)
: (
   c*eg*x29*x30 + x10*x20*x30 - 2*x26*std::atan(x28/eg)
));
        double eps2 = ((x16) ? (
   a*std::pow(x15, 2)*x18/(x0*x1 + std::pow(x9, 2))
)
: (
   0
));
        er_sum += eps1;
        ei_sum += eps2;
    }
    return std::sqrt(er_sum - 1i * ei_sum);
}
