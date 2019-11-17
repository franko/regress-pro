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

const auto x0 = a*c;
const auto x1 = std::pow(en, 2);
const auto x2 = std::pow(c, 2);
const auto x3 = std::pow(eg, 2);
const auto x4 = x2*x3;
const auto x5 = std::pow(e0, 2);
const auto x6 = -x3 + x5;
const auto x7 = x4 + std::pow(x6, 2);
const auto x8 = std::sqrt(x7);
const auto x9 = std::log(4*x1/x8);
const auto x10 = 2*e0;
const auto x11 = M_1_PI;
const auto x12 = -x2;
const auto x13 = 4*x5;
const auto x14 = x12 + x13;
const auto x15 = (1.0/4.0)*x2;
const auto x16 = (1.0/2.0)*x2;
const auto x17 = -x5;
const auto x18 = x1 + x17;
const auto x19 = x16 + x18;
const auto x20 = x14*x15 + std::pow(x19, 2);
const auto x21 = x11/x20;
const auto x22 = x10*x21;
const auto x23 = en*x22;
const auto x24 = x23*x9;
const auto x25 = en > eg - eg_delta && en < eg + eg_delta;
const auto x26 = 1.0/en;
const auto x27 = c*e0;
const auto x28 = x26*x27;
const auto x29 = a*x28;
const auto x30 = eg + en;
const auto x31 = std::log(x30);
const auto x32 = -eg + en;
const auto x33 = eg < en;
const auto x34 = eg - en;
const auto x35 = ((x33) ? (
   std::log(x32)
)
: (
   std::log(x34)
));
const auto x36 = -x31 + x35;
const auto x37 = x1 + x3;
const auto x38 = x21*x37;
const auto x39 = x36*x38;
const auto x40 = x31 + x35 - std::log(x8);
const auto x41 = 2*eg;
const auto x42 = x21*x41;
const auto x43 = x40*x42;
const auto x44 = x27*x43;
const auto x45 = std::sqrt(x14);
const auto x46 = 1.0/x45;
const auto x47 = 1.0/c;
const auto x48 = 2*x3;
const auto x49 = x12 - x48 + 2*x5;
const auto x50 = x46*x49;
const auto x51 = 2*std::atan(x47*x50) + M_PI;
const auto x52 = x46*x51;
const auto x53 = x19*x52;
const auto x54 = a*e0;
const auto x55 = x42*x54;
const auto x56 = -x41 + x45;
const auto x57 = x41 + x45;
const auto x58 = std::atan(x47*x56) - std::atan(x47*x57) + M_PI;
const auto x59 = x3 + x5;
const auto x60 = x18*x59 + x4;
const auto x61 = 1.0/e0;
const auto x62 = x21*x61;
const auto x63 = x60*x62;
const auto x64 = x58*x63;
const auto x65 = (1.0/2.0)*c;
const auto x66 = 3*x3 + x5;
const auto x67 = x1*(x17 + x3) + x4 - x5*x66;
const auto x68 = eg*x45;
const auto x69 = x59 + x68;
const auto x70 = x59 - x68;
const auto x71 = 1.0/x70;
const auto x72 = std::log(x69*x71);
const auto x73 = x46*x62*x72;
const auto x74 = x67*x73;
const auto x75 = x65*x74;
const auto x76 = c < 1.9999999999*e0;
const auto x77 = 1.0/x59;
const auto x78 = c*x19;
const auto x79 = x77*x78;
const auto x80 = a*x63;
const auto x81 = 1.0/eg;
const auto x82 = std::atan(x65*x81);
const auto x83 = 2*x82;
const auto x84 = c*eg;
const auto x85 = x62*x77;
const auto x86 = x67*x85;
const auto x87 = x84*x86;
const auto x88 = std::pow(x32, 2);
const auto x89 = x1*x2 + std::pow(x18, 2);
const auto x90 = 1.0/x89;
const auto x91 = x88*x90;
const auto x92 = x28*x91;
const auto x93 = eg*x2;
const auto x94 = x41*x6;
const auto x95 = 1.0/x7;
const auto x96 = x23*x95;
const auto x97 = x29*x36;
const auto x98 = 1.0/x30;
const auto x99 = ((x33) ? (
   -1/x32
)
: (
   1.0/x34
));
const auto x100 = x0*x40;
const auto x101 = a*x42;
const auto x102 = x101*x27;
const auto x103 = a*x22;
const auto x104 = x2*x41;
const auto x105 = x104 + x18*x41;
const auto x106 = a*x58;
const auto x107 = x106*x62;
const auto x108 = 1.0/x14;
const auto x109 = 1.0/x2;
const auto x110 = x21/(x108*x109*std::pow(x49, 2) + 1);
const auto x111 = eg*x5;
const auto x112 = x1*x41 + x104 - 6*x111;
const auto x113 = a*x73;
const auto x114 = x113*x65;
const auto x115 = 1.0/(x109*std::pow(x56, 2) + 1);
const auto x116 = 2*x47;
const auto x117 = 1.0/(x109*std::pow(x57, 2) + 1);
const auto x118 = x69/std::pow(x70, 2);
const auto x119 = x46*x65;
const auto x120 = x62*x67;
const auto x121 = a*x119*x120*x70/x69;
const auto x122 = std::pow(x59, -2);
const auto x123 = 4*e0;
const auto x124 = x123*x19;
const auto x125 = x0*x21;
const auto x126 = a*x83;
const auto x127 = x126*x62;
const auto x128 = a*x84;
const auto x129 = x128*x85;
const auto x130 = 1.0/x3;
const auto x131 = 1.0/(x130*x15 + 1);
const auto x132 = x0*x67;
const auto x133 = x122*x132;
const auto x134 = e0*x42;
const auto x135 = 2*en;
const auto x136 = x6*x95;
const auto x137 = x10*x2;
const auto x138 = x124 - x137;
const auto x139 = x11/std::pow(x20, 2);
const auto x140 = x138*x139;
const auto x141 = en*x0*x10*x9;
const auto x142 = x13*x21;
const auto x143 = x26*x39;
const auto x144 = x37*x97;
const auto x145 = x140*x41;
const auto x146 = a*x27*x40;
const auto x147 = a*eg;
const auto x148 = x142*x147;
const auto x149 = std::pow(x14, -3.0/2.0);
const auto x150 = x149*x21;
const auto x151 = x10*x18 - x10*x59;
const auto x152 = x106*x60;
const auto x153 = x21/x5;
const auto x154 = -2*std::pow(e0, 3) - x1*x10 - x10*x66;
const auto x155 = x145*x54;
const auto x156 = x139*x61;
const auto x157 = x152*x156;
const auto x158 = x153*x67;
const auto x159 = a*x72;
const auto x160 = x119*x159;
const auto x161 = x123*x46;
const auto x162 = x161*x47;
const auto x163 = x149*x49;
const auto x164 = x110*x124*x147*x46;
const auto x165 = x156*x67;
const auto x166 = x160*x165;
const auto x167 = eg*x161;
const auto x168 = x128*x77;
const auto x169 = x126*x60;
const auto x170 = x156*x169;
const auto x171 = x165*x168;
const auto x172 = (1.0/2.0)*std::pow(c, 3) - x14*x65 - 2*x78;
const auto x173 = x139*x172;
const auto x174 = std::pow(eg, 3);
const auto x175 = a*x137;
const auto x176 = x173*x41;
const auto x177 = x0*x62;
const auto x178 = x176*x54;
const auto x179 = -x46;
const auto x180 = x46*x84;
const auto x181 = x54*x77;
const auto x182 = x26*x91;
const auto x183 = x88/std::pow(x89, 2);

        er_sum += ((x25) ? (
   x0*x24
)
: (
   a*x44 - x29*x39
)) + ((x76) ? (
   -a*x64 + a*x75 + x53*x55
)
: (
   a*x87 + x55*x79 - x80*x83
));
        ei_sum += ((x33) ? (
   a*x92
)
: (
   0
));

        const double d_eg_real = ((x25) ? (
   x0*x96*(-x93 + x94)
)
: (
   x100*x22 + x102*(-x95*(x93 - x94) + x98 + x99) - x29*x38*(-x98 + x99) - x42*x97
)) + ((x76) ? (
   x103*x53 - x105*x107 - 16*x108*x110*x19*x3*x47*x54 + x112*x114 + x121*(x118*x56 + x57*x71) - x80*(-x115*x116 - x116*x117)
)
: (
   x0*x130*x131*x63 + x103*x79 - x105*x127 + x112*x129 - x122*x124*x125*x3 + x132*x85 - x133*x48*x62
));
        const double d_eg_imag = ((x33) ? (
   x29*x90*(-x135 + x41)
)
: (
   0
));
        const cmpl d_eg = cmpl(d_eg_real, -d_eg_imag);
        pd->at(TL_EG_OFFS) += d_eg;

        const double d_a_real = ((x25) ? (
   c*x24
)
: (
   -x28*x39 + x44
)) + ((x76) ? (
   x134*x53 - x64 + x75
)
: (
   x134*x79 - x63*x83 + x87
));
        const double d_a_imag = ((x33) ? (
   x92
)
: (
   0
));
        const cmpl d_a = cmpl(d_a_real, -d_a_imag);

        const double d_e0_real = ((x25) ? (
   -en*x125*x13*x136 + x125*x135*x9 + x140*x141
)
: (
   -x0*x143 + x100*x42 - x128*x136*x142 - x140*x144 + x145*x146
)) + ((x76) ? (
   -8*a*x111*x150*x19*x51 + x101*x53 - x107*x151 + x114*x154 + x121*(x118*(-x10 + x167) + x71*(x10 + x167)) - 2*x132*x150*x72 - x138*x157 + x138*x166 - x148*x52 + x152*x153 + x155*x53 - x158*x160 + x164*(-x123*x163*x47 + x162) - x80*(x115*x162 - x117*x162)
)
: (
   x101*x79 - x122*x148*x78 - x127*x151 + x129*x154 - x133*x42 - x138*x170 + x138*x171 - x142*x168 + x153*x169 + x155*x79 - x158*x168
));
        const double d_e0_imag = ((x33) ? (
   x0*x13*x18*x183*x26 + x0*x182
)
: (
   0
));
        const cmpl d_e0 = cmpl(d_e0_real, -d_e0_imag);

        const double d_c_real = ((x25) ? (
   a*x24 - a*x4*x96 + x141*x173
)
: (
   -x143*x54 - x144*x173 + x146*x176 - x174*x175*x21*x95 + x43*x54
)) + ((x76) ? (
   (1.0/2.0)*a*x74 + x102*x52 + x113*x4 + x120*x149*x159*x16 + x121*(-x118*x180 - x180*x71) + x149*x51*x55*x78 - x157*x172 + x164*(-x109*x50 + x163 - 2*x46) + x166*x172 - x177*x48*x58 + x178*x53 - x80*(x115*(-x109*x56 + x179) - x117*(-x109*x57 + x179))
)
: (
   2*a*x174*x2*x85 + x104*x181*x21 - x131*x80*x81 + x147*x86 - x170*x172 + x171*x172 - 4*x177*x3*x82 + x178*x79 + x181*x19*x42
));
        const double d_c_imag = ((x33) ? (
   -en*x175*x183 + x182*x54
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
const auto x0 = std::pow(c_p, 4);
const auto x1 = std::pow(e0_p, 2);
const auto x2 = 4*std::pow(e0_p, 4) + x0;
const auto x3 = std::sqrt(x2);
const auto x4 = -2*x1 + x3;
const auto x5 = std::sqrt(x4);
const auto x6 = 1.0/x5;
const auto x7 = M_SQRT2;
const auto x8 = (1.0/4.0)*x7;
const auto x9 = a_p/std::pow(x4, 3.0/2.0);
const auto x10 = std::pow(c_p, 3);
const auto x11 = x7/std::pow(x2, 3.0/4.0);
const auto x12 = 1.0/x3;
const double dp11 = x0*x6*x8/std::pow(x2, 1.0/4.0);
const double dp12 = (1.0/2.0)*e0_p*x0*x7*x9*(x1*(4*x1 - 2*x3) + x3*x4)/std::pow(x2, 5.0/4.0);
const double dp13 = x10*x8*x9*(-x0*std::pow(x2, 3.0/2.0) - x0*x2*x4 + 4*std::pow(x2, 2)*x4)/std::pow(x2, 9.0/4.0);
const double dp21 = 0;
const double dp22 = 2*std::pow(e0_p, 3)*x11;
const double dp23 = (1.0/2.0)*x10*x11;
const double dp31 = 0;
const double dp32 = -2*e0_p*x12*x5;
const double dp33 = x10*x12*x6;
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
