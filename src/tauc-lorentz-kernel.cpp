cmpl tauc_lorentz_n_value_deriv(const disp_t *d, double lambda, cmpl_vector *pd) {
    const struct disp_fb *fb = &d->disp.fb;
    const int nb = fb->n;
    const double en = TL_EV_NM / lambda;
    const double eg_delta = 1.0e-4;

    /* If Eg is negative use zero instead. A negative Eg is not meaningful. */
    const double eg = (fb->eg >= 0 ? fb->eg : 0.0);

    pd->at(TL_NINF_OFFS) = cmpl(1.0, 0.0);
    pd->at(TL_EG_OFFS) = 0;

    double er_sum = fb->n_inf, ei_sum = 0.0;
    for (int osc_index = TL_OSC_OFFS, k = 0; k < nb; k++, osc_index += TL_NB_OSC_PARAMS) {
        double a, e0, c;
        oscillator_parameters(fb->form, fb->osc + k, a, e0, c);

const auto x0 = 2*e0*eg;
const auto x1 = 1.0/M_PI;
const auto x2 = std::pow(c, 2);
const auto x3 = -x2;
const auto x4 = std::pow(e0, 2);
const auto x5 = x3 + 4*x4;
const auto x6 = std::sqrt(x5);
const auto x7 = 1.0/x6;
const auto x8 = std::pow(en, 2);
const auto x9 = -x4;
const auto x10 = x8 + x9;
const auto x11 = x10 + (1.0L/2.0L)*x2;
const auto x12 = std::pow(x11, 2) + (1.0L/4.0L)*x2*x5;
const auto x13 = 1.0/x12;
const auto x14 = 1.0/c;
const auto x15 = std::pow(eg, 2);
const auto x16 = -2*x15 + x3 + 2*x4;
const auto x17 = x16*x7;
const auto x18 = 2*atan(x14*x17) + M_PI;
const auto x19 = a*x1*x11*x13*x18*x7;
const auto x20 = x15*x2;
const auto x21 = x15 + x4;
const auto x22 = x10*x21 + x20;
const auto x23 = 1.0/e0;
const auto x24 = 2*eg;
const auto x25 = -x24 + x6;
const auto x26 = x24 + x6;
const auto x27 = atan(x14*x25) - atan(x14*x26) + M_PI;
const auto x28 = a*x1*x13*x23*x27;
const auto x29 = x15 + x9;
const auto x30 = 3*x15 + x4;
const auto x31 = x20 + x29*x8 - x30*x4;
const auto x32 = eg*x6;
const auto x33 = x21 + x32;
const auto x34 = x21 - x32;
const auto x35 = 1.0/x34;
const auto x36 = log(x33*x35);
const auto x37 = (1.0L/2.0L)*a*c*x1*x13*x23*x36*x7;
const auto x38 = 2*e0*en;
const auto x39 = -x15 + x4;
const auto x40 = x20 + std::pow(x39, 2);
const auto x41 = std::sqrt(x40);
const auto x42 = log(4*x8/x41);
const auto x43 = a*c*x1*x13*x42;
const auto x44 = en > eg - eg_delta && en < eg + eg_delta;
const auto x45 = x15 + x8;
const auto x46 = 1.0/en;
const auto x47 = eg + en;
const auto x48 = log(x47);
const auto x49 = -eg + en;
const auto x50 = en > eg;
const auto x51 = eg - en;
const auto x52 = ((x50) ? (
   log(x49)
)
: (
   log(x51)
));
const auto x53 = -x48 + x52;
const auto x54 = a*c*x1*x13*x46*x53;
const auto x55 = x45*x54;
const auto x56 = x48 + x52 - log(x41);
const auto x57 = a*c*x1*x13*x56;
const auto x58 = std::pow(x49, 2);
const auto x59 = std::pow(x10, 2) + x2*x8;
const auto x60 = 1.0/x59;
const auto x61 = a*x46*x58*x60;
const auto x62 = c*x61;
const auto x63 = 2*a*e0*x1*x13*x18;
const auto x64 = x2*x24;
const auto x65 = a*e0*x1*x13;
const auto x66 = 1.0/x5;
const auto x67 = 1.0/x2;
const auto x68 = 1.0/(std::pow(x16, 2)*x66*x67 + 1);
const auto x69 = 2*x14;
const auto x70 = 1.0/(std::pow(x25, 2)*x67 + 1);
const auto x71 = 1.0/(std::pow(x26, 2)*x67 + 1);
const auto x72 = a*x1*x13*x22*x23;
const auto x73 = x33/std::pow(x34, 2);
const auto x74 = (1.0L/2.0L)*a*c*x1*x13*x23*x31*x34*x7/x33;
const auto x75 = eg*x2;
const auto x76 = x24*x39;
const auto x77 = 1.0/x40;
const auto x78 = 1.0/x47;
const auto x79 = ((x50) ? (
   -1/x49
)
: (
   1.0/x51
));
const auto x80 = 2*e0;
const auto x81 = x1*x13;
const auto x82 = 2*e0*eg*x11*x18*x7;
const auto x83 = x22*x23*x27;
const auto x84 = (1.0L/2.0L)*c*x23*x31*x36*x7;
const auto x85 = 2*c*e0*en*x42;
const auto x86 = c*e0*x45*x46*x53;
const auto x87 = 2*c*e0*eg*x56;
const auto x88 = std::pow(x5, -3.0L/2.0L);
const auto x89 = a*x1*x13*x18*x88;
const auto x90 = a*x1*x13/x4;
const auto x91 = std::pow(x12, -2);
const auto x92 = a*x1*x91*(4*e0*x11 - x2*x80);
const auto x93 = x31*x36*x88;
const auto x94 = 4*e0*x7;
const auto x95 = x14*x94;
const auto x96 = x16*x88;
const auto x97 = 4*a*e0*eg*x1*x11*x13*x68*x7;
const auto x98 = eg*x94;
const auto x99 = 2*en;
const auto x100 = 4*a*c*x1*x13*x39*x4*x77;
const auto x101 = c*eg*x7;
const auto x102 = a*x1*x13*x23*x36*x7;
const auto x103 = a*x1*x91*((1.0L/2.0L)*std::pow(c, 3) - 2*c*x11 - 1.0L/2.0L*c*x5);
const auto x104 = -x7;
const auto x105 = a*x1*x13*x77;
const auto x106 = c*e0*x46*x60;
const auto x107 = std::pow(x59, -2);

        er_sum += x0*x19 - x22*x28 + x31*x37 + ((x44) ? (
   x38*x43
)
: (
   -e0*x55 + x0*x57
));
        ei_sum += ((x50) ? (
   e0*x62
)
: (
   0
));

        const double d_eg_real = -16*x11*x14*x15*x65*x66*x68 + x11*x63*x7 - x28*(x10*x24 + x64) + x37*(-6*eg*x4 + x24*x8 + x64) - x72*(-x69*x70 - x69*x71) + x74*(x25*x73 + x26*x35) + ((x44) ? (
   2*a*c*e0*en*x1*x13*x77*(-x75 + x76)
)
: (
   2*a*c*e0*eg*x1*x13*(-x77*(x75 - x76) + x78 + x79) - c*x45*x46*x65*(-x78 + x79) - x0*x54 + x57*x80
));
        const double d_eg_imag = ((x50) ? (
   a*x106*(x24 - x99)
)
: (
   0
));
        const cmpl d_eg = cmpl(d_eg_real, -d_eg_imag);
        pd->at(TL_EG_OFFS) += d_eg;

        const double d_a_real = x81*x82 - x81*x83 + x81*x84 + ((x44) ? (
   x81*x85
)
: (
   -x81*x86 + x81*x87
));
        const double d_a_imag = ((x50) ? (
   x106*x58
)
: (
   0
));
        const cmpl d_a = cmpl(d_a_real, -d_a_imag);

        const double d_e0_real = -2*a*c*x1*x13*x93 - 4*a*eg*x1*x13*x18*x4*x7 - 1.0L/2.0L*c*x31*x36*x7*x90 - 8*eg*x11*x4*x89 + x19*x24 + x22*x27*x90 - x28*(x10*x80 - x21*x80) + x37*(-2*std::pow(e0, 3) - x30*x80 - x8*x80) - x72*(x70*x95 - x71*x95) + x74*(x35*(x80 + x98) + x73*(-x80 + x98)) + x82*x92 - x83*x92 + x84*x92 + x97*(-4*e0*x14*x96 + x95) + ((x44) ? (
   -en*x100 + x43*x99 + x85*x92
)
: (
   -eg*x100 + x24*x57 - x55 - x86*x92 + x87*x92
));
        const double d_e0_imag = ((x50) ? (
   4*a*c*x10*x107*x4*x46*x58 + x62
)
: (
   0
));
        const cmpl d_e0 = cmpl(d_e0_real, -d_e0_imag);

        const double d_c_real = (1.0L/2.0L)*a*x1*x13*x2*x23*x93 + 2*c*e0*eg*x11*x89 - 2*c*x15*x28 + x101*x63 + x102*x20 + x102*((1.0L/2.0L)*x15*x2 + (1.0L/2.0L)*x29*x8 - 1.0L/2.0L*x30*x4) + x103*x82 - x103*x83 + x103*x84 - x72*(x70*(x104 - x25*x67) - x71*(x104 - x26*x67)) + x74*(-x101*x35 - x101*x73) + x97*(-x17*x67 - 2*x7 + x96) + ((x44) ? (
   a*x1*x13*x38*x42 - 2*e0*en*x105*x15*x2 + x103*x85
)
: (
   2*a*e0*eg*x1*x13*x56 - a*e0*x1*x13*x45*x46*x53 - 2*e0*std::pow(eg, 3)*x105*x2 - x103*x86 + x103*x87
));
        const double d_c_imag = ((x50) ? (
   -2*a*e0*en*x107*x2*x58 + e0*x61
)
: (
   0
));
        const cmpl d_c = cmpl(d_c_real, -d_c_imag);

        if (fb->form == TAUC_LORENTZ_STANDARD) {
            pd->at(osc_index + TL_AL_OFFS) = d_a;
            pd->at(osc_index + TL_E0_OFFS) = d_e0;
            pd->at(osc_index + TL_C_OFFS ) = d_c;
        } else {
            const double a_p = fb->osc[k].a, e0_p = fb->osc[k].b, c_p = fb->osc[k].c;
const auto x0 = std::sqrt(2);
const auto x1 = std::pow(c_p, 4);
const auto x2 = 4*std::pow(e0_p, 4) + x1;
const auto x3 = std::pow(e0_p, 2);
const auto x4 = std::sqrt(x2);
const auto x5 = -2*x3 + x4;
const auto x6 = std::sqrt(x5);
const auto x7 = 1.0/x6;
const auto x8 = a_p/std::pow(x5, 3.0L/2.0L);
const auto x9 = std::pow(c_p, 3);
const auto x10 = x0/std::pow(x2, 3.0L/4.0L);
const auto x11 = 1.0/x4;
const double dp11 = (1.0L/4.0L)*x0*x1*x7/std::pow(x2, 1.0L/4.0L);
const double dp12 = (1.0L/2.0L)*e0_p*x0*x1*x8*(x3*(4*x3 - 2*x4) + x4*x5)/std::pow(x2, 5.0L/4.0L);
const double dp13 = (1.0L/4.0L)*x0*x8*x9*(-x1*std::pow(x2, 3.0L/2.0L) - x1*x2*x5 + 4*std::pow(x2, 2)*x5)/std::pow(x2, 9.0L/4.0L);
const double dp21 = 0;
const double dp22 = 2*std::pow(e0_p, 3)*x10;
const double dp23 = (1.0L/2.0L)*x10*x9;
const double dp31 = 0;
const double dp32 = -2*e0_p*x11*x6;
const double dp33 = x11*x7*x9;
            pd->at(osc_index + TL_AL_OFFS) = d_a * dp11 + d_e0 * dp21 + d_c * dp31;
            pd->at(osc_index + TL_E0_OFFS) = d_a * dp12 + d_e0 * dp22 + d_c * dp32;
            pd->at(osc_index + TL_C_OFFS ) = d_a * dp13 + d_e0 * dp23 + d_c * dp33;
        }
    }

    cmpl nn = sqrt(cmpl(er_sum, -ei_sum));

    for (int i = 0; i < TL_NB_GLOBAL_PARAMS + nb * TL_NB_OSC_PARAMS; i++) {
        pd->at(i) = cmpl(1, 0) / (2 * nn) * pd->at(i);
    }

    return nn;
}
