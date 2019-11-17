/* Generated with sympy using the command: tauc-lorentz-sympy.py ../src/tauc-lorentz-template.cpp */
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
const auto x5 = -x2 + x4;
const auto x6 = std::sqrt(x3 + std::pow(x5, 2));
const auto x7 = 4*x4;
const auto x8 = x1 - 2*x4;
const auto x9 = 2*x0 + x8;
const auto x10 = a/(M_PI*(x1*(x1 - x7) - std::pow(x9, 2)));
const auto x11 = c*e0;
const auto x12 = 1.0/en;
const auto x13 = std::log(eg + en);
const auto x14 = eg < en;
const auto x15 = eg - en;
const auto x16 = ((x14) ? (
   std::log(-eg + en)
)
: (
   std::log(x15)
));
const auto x17 = 2*eg;
const auto x18 = 4*x10;
const auto x19 = std::sqrt(-x1 + x7);
const auto x20 = 1.0/x19;
const auto x21 = -M_PI;
const auto x22 = 1.0/c;
const auto x23 = e0*x9;
const auto x24 = 1.0/e0;
const auto x25 = x2 + x4;
const auto x26 = -x0 + x4;
const auto x27 = 2*x24*(-x25*x26 + x3);
const auto x28 = eg*x19;
const auto x29 = c*x24*(x0*x5 - x3 + x4*(3*x2 + x4));
const auto x30 = eg/x25;

        double eps1 = ((c < 1.9999999999*e0) ? (
   2*x10*(x17*x20*x23*(x21 + 2*std::atan(x20*x22*(2*x2 + x8))) + x20*x29*std::log((x25 + x28)/(x25 - x28)) - x27*(x21 + std::atan(x22*(x17 - x19)) + std::atan(x22*(x17 + x19))))
)
: (
   x18*(-c*x23*x30 + x27*std::atan((1.0/2.0)*c/eg) + x29*x30)
)) + ((en > eg - eg_delta && en < eg + eg_delta) ? (
   -8*en*x10*x11*std::log(4*x0/x6)
)
: (
   x11*x18*(x12*(x0 + x2)*(-x13 + x16) - x17*(x13 + x16 - std::log(x6)))
));
        double eps2 = ((x14) ? (
   a*x11*x12*std::pow(x15, 2)/(x0*x1 + std::pow(x26, 2))
)
: (
   0
));
        er_sum += eps1;
        ei_sum += eps2;
    }
    return std::sqrt(er_sum - 1i * ei_sum);
}

