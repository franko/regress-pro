/* Generated with sympy using the command: tauc-lorentz-diff-sympy.py
 * ../src/tauc-lorentz-diff-template.cpp */
cmpl tauc_lorentz_n_value_deriv(const disp_t *d, double lambda,
                                cmpl_vector *pd) {
    const struct disp_fb *fb = &d->disp.fb;
    const int nb = fb->n;
    const double en = TL_EV_NM / lambda;
    const double eg_delta = 1.0e-4;

    /* If Eg is negative use zero instead. A negative Eg is not meaningful. */
    const double eg = (fb->eg >= 0 ? fb->eg : 0.0);

    pd->at(TL_NINF_OFFS) = cmpl(1.0, 0.0);
    pd->at(TL_EG_OFFS) = 0;

    double er_sum = fb->n_inf, ei_sum = 0.0;
    for (int osc_index = TL_OSC_OFFS, k = 0; k < nb;
         k++, osc_index += TL_NB_OSC_PARAMS) {
        double a, e0, c;
        oscillator_parameters(fb->form, fb->osc + k, a, e0, c);

        const auto x0 = std::pow(en, 2);
        const auto x1 = std::pow(c, 2);
        const auto x2 = std::pow(eg, 2);
        const auto x3 = x1 * x2;
        const auto x4 = std::pow(e0, 2);
        const auto x5 = -x2 + x4;
        const auto x6 = x3 + std::pow(x5, 2);
        const auto x7 = std::sqrt(x6);
        const auto x8 = std::log(4 * x0 / x7);
        const auto x9 = 8 * x8;
        const auto x10 = M_1_PI;
        const auto x11 = 4 * x4;
        const auto x12 = x1 - x11;
        const auto x13 = x1 * x12;
        const auto x14 = 2 * x4;
        const auto x15 = x1 - x14;
        const auto x16 = 2 * x0 + x15;
        const auto x17 = std::pow(x16, 2);
        const auto x18 = 1.0 / (x13 - x17);
        const auto x19 = x10 * x18;
        const auto x20 = c * e0;
        const auto x21 = x19 * x20;
        const auto x22 = en * x21 * x9;
        const auto x23 = en > eg - eg_delta && en < eg + eg_delta;
        const auto x24 = 4 * x19;
        const auto x25 = a * x24;
        const auto x26 = eg + en;
        const auto x27 = std::log(x26);
        const auto x28 = eg < en;
        const auto x29 = eg - en;
        const auto x30 = ((x28) ? (std::log(-eg + en)) : (std::log(x29)));
        const auto x31 = -x27 + x30;
        const auto x32 = 1.0 / en;
        const auto x33 = x0 + x2;
        const auto x34 = x32 * x33;
        const auto x35 = x31 * x34;
        const auto x36 = std::log(x7);
        const auto x37 = x27 + x30 - x36;
        const auto x38 = 2 * eg;
        const auto x39 = x37 * x38;
        const auto x40 = x20 * (x35 - x39);
        const auto x41 = -x1 + x11;
        const auto x42 = std::sqrt(x41);
        const auto x43 = 1.0 / x42;
        const auto x44 = x38 * x43;
        const auto x45 = -M_PI;
        const auto x46 = 2 * x2;
        const auto x47 = x15 + x46;
        const auto x48 = 1.0 / c;
        const auto x49 = x43 * x48;
        const auto x50 = 2 * std::atan(x47 * x49);
        const auto x51 = x45 + x50;
        const auto x52 = e0 * x16;
        const auto x53 = x51 * x52;
        const auto x54 = x2 + x4;
        const auto x55 = -x0;
        const auto x56 = x4 + x55;
        const auto x57 = x3 - x54 * x56;
        const auto x58 = 2 * x57;
        const auto x59 = 1.0 / e0;
        const auto x60 = x38 + x42;
        const auto x61 = std::atan(x48 * x60);
        const auto x62 = -x42;
        const auto x63 = x38 + x62;
        const auto x64 = std::atan(x48 * x63);
        const auto x65 = x59 * (x45 + x61 + x64);
        const auto x66 = 3 * x2;
        const auto x67 = x4 * (x4 + x66);
        const auto x68 = x0 * x5;
        const auto x69 = x59 * (-x3 + x67 + x68);
        const auto x70 = eg * x42;
        const auto x71 = x54 + x70;
        const auto x72 = x71 / (x54 - x70);
        const auto x73 = std::log(x72);
        const auto x74 = x43 * x73;
        const auto x75 = x69 * x74;
        const auto x76 = 2 * x19;
        const auto x77 = x76 * (c * x75 + x44 * x53 - x58 * x65);
        const auto x78 = c < 1.9999999999 * e0;
        const auto x79 = 1.0 / x54;
        const auto x80 = c * x79;
        const auto x81 = x16 * x80;
        const auto x82 = e0 * x81;
        const auto x83 = 1.0 / eg;
        const auto x84 = std::atan((1.0 / 2.0) * c * x83);
        const auto x85 = x58 * x84;
        const auto x86 = x69 * x80;
        const auto x87 = x24 * (-eg * x82 + eg * x86 + x59 * x85);
        const auto x88 = x0 * x1;
        const auto x89 = 1.0 / (std::pow(x56, 2) + x88);
        const auto x90 = std::pow(x29, 2) * x89;
        const auto x91 = x32 * x90;
        const auto x92 = x20 * x91;
        const auto x93 = 1.0 / x6;
        const auto x94 = eg * x93;
        const auto x95 = x47 * x94;
        const auto x96 = 8 * en;
        const auto x97 = 1.0 / x26;
        const auto x98 = 1.0 / x29;
        const auto x99 = 4 * x65;
        const auto x100 = x0 + x1;
        const auto x101 = eg * (x100 - x4);
        const auto x102 = 2 * x43;
        const auto x103 = 1.0 / x12;
        const auto x104 = 1.0 / x1;
        const auto x105 = 1.0 / (-x103 * x104 * std::pow(x47, 2) + 1);
        const auto x106 = x105 * x52;
        const auto x107 = 16 * x48;
        const auto x108 = x59 * (x100 - 3 * x4);
        const auto x109 = 1.0 / (x104 * std::pow(x60, 2) + 1);
        const auto x110 = 1.0 / (x104 * std::pow(x63, 2) + 1);
        const auto x111 = 4 * x59;
        const auto x112 = x111 * x57;
        const auto x113 = 1.0 / x71;
        const auto x114 = -x38;
        const auto x115 = c * x69;
        const auto x116 = a * x76;
        const auto x117 = x111 * x84;
        const auto x118 = std::pow(x54, -2);
        const auto x119 = x118 * x46;
        const auto x120 = 1.0 / x2;
        const auto x121 = x112 / (x1 * x120 + 4);
        const auto x122 = 1.0 / (-x13 + x17);
        const auto x123 = x122 * x56;
        const auto x124 = 16 * x123;
        const auto x125 = x124 * x4;
        const auto x126 = x10 * x122;
        const auto x127 = a * x126;
        const auto x128 = x127 * x96;
        const auto x129 = eg * x4;
        const auto x130 = 32 * x123;
        const auto x131 = x129 * x130;
        const auto x132 = -x35 + x39;
        const auto x133 = 4 * x127;
        const auto x134 = M_PI - x50;
        const auto x135 = 8 * x129 * x134;
        const auto x136 = x134 * x16;
        const auto x137 = -x61 - x64 + M_PI;
        const auto x138 = 4 * x14 + 4 * x2 + 4 * x55;
        const auto x139 = std::pow(x41, -3.0 / 2.0);
        const auto x140 = x0 + x14 + x66;
        const auto x141 = 2 * x74;
        const auto x142 = 1.0 / x4;
        const auto x143 = x114 + x42;
        const auto x144 = x3 - x67 - x68;
        const auto x145 = c * x144;
        const auto x146 = x139 * x73;
        const auto x147 = 1.0 / x41;
        const auto x148 = x147 * x47;
        const auto x149 = x145 * x74;
        const auto x150 = x130 * x57;
        const auto x151 = 2 * a;
        const auto x152 = eg * x80;
        const auto x153 = eg * x81;
        const auto x154 = c * eg;
        const auto x155 = x144 * x152;
        const auto x156 = x122 * x88;
        const auto x157 = 2 * x1;
        const auto x158 = std::pow(eg, 3) * x157;
        const auto x159 = 4 * eg;
        const auto x160 = c * x2;
        const auto x161 = x1 * x69;
        const auto x162 = 16 * x0 * x18;
        const auto x163 = c * x162 * x57;
        const auto x164 = 8 * x18 * x88;
        const auto x165 = eg * x79;
        const auto x166 = x165 * x52;
        const auto x167 = x165 * x69;

        er_sum += ((x23) ? (-a * x22) : (x25 * x40)) +
                  ((x78) ? (a * x77) : (a * x87));
        ei_sum += ((x28) ? (a * x92) : (0));

        const double d_eg_real =
            ((x78) ? (x116 *
                      (-c * x108 * x44 * x73 - x101 * x99 + x102 * x53 -
                       x103 * x106 * x107 * x2 - x112 * x48 * (x109 + x110) -
                       x113 * x115 * x43 * (x114 + x62 + x63 * x72)))
                   : (x25 * (c * x119 * x52 - c * x120 * x121 + x101 * x117 -
                             x108 * x46 * x80 - x115 * x119 - x82 + x86))) +
            ((x23)
                 ? (a * x21 * x95 * x96)
                 : (x20 * x25 *
                    (-2 * x27 - 2 * x30 + x31 * x32 * x38 - x34 * (x97 - x98) +
                     2 * x36 - x38 * (-x95 + x97 + x98))));
        const double d_eg_imag = ((x28) ? (x151 * x20 * x29 * x32 * x89) : (0));
        const cmpl d_eg = cmpl(d_eg_real, -d_eg_imag);
        pd->at(TL_EG_OFFS) += d_eg;

        const double d_a_real =
            ((x23) ? (-x22) : (x24 * x40)) + ((x78) ? (x77) : (x87));
        const double d_a_imag = ((x28) ? (x92) : (0));
        const cmpl d_a = cmpl(d_a_real, -d_a_imag);

        const double d_e0_real =
            ((x23) ? (c * x128 * (-x125 * x8 - x14 * x5 * x93 + x8))
                   : (c * x133 *
                      (-x11 * x5 * x94 + x125 * x35 - x131 * x37 + x132))) +
            ((x78)
                 ? (x126 * x151 *
                    (-c * x140 * x141 +
                     x102 * x113 * x145 * (x44 + x72 * (x44 - 1) + 1) +
                     x105 * x107 * x129 * x147 * x16 * (x148 + 1) -
                     x124 * x149 - x131 * x136 * x43 - x135 * x139 * x16 -
                     x135 * x43 + x136 * x44 + x137 * x138 + x137 * x142 * x58 +
                     x137 * x150 - x142 * x149 - 4 * x145 * x146 +
                     8 * x49 * x57 *
                         (x109 - 1 / (x104 * std::pow(x143, 2) + 1))))
                 : (x133 *
                    (-x11 * x152 - x118 * x14 * x154 * x16 - x118 * x145 * x38 -
                     x124 * x155 - x125 * x153 + x138 * x84 - x140 * x38 * x80 -
                     x142 * x155 + x142 * x85 + x150 * x84 + x153)));
        const double d_e0_imag =
            ((x28) ? (a * c * x91 * (-x11 * x56 * x89 + 1)) : (0));
        const cmpl d_e0 = cmpl(d_e0_real, -d_e0_imag);

        const double d_c_real =
            ((x78) ? (x116 * (c * x139 * x38 * x53 -
                              eg * x113 * x147 * x161 * (x72 + 1) +
                              x106 * x147 * x159 * (-x104 * x47 + x148 + 2) -
                              x141 * x3 * x59 + x146 * x161 +
                              x154 * x162 * x43 * x53 + x159 * x20 * x43 * x51 -
                              x160 * x99 - x163 * x65 + x164 * x75 -
                              x58 * x59 *
                                  (-x109 * (x104 * x60 + x43) +
                                   x110 * (x104 * x143 + x43)) +
                              x75))
                   : (x25 * (-e0 * x1 * x38 * x79 + x117 * x160 + x121 * x83 -
                             x158 * x59 * x79 + x163 * x59 * x84 - x164 * x166 +
                             x164 * x167 - x166 + x167))) +
            ((x23) ? (e0 * x128 * (-x156 * x9 - x3 * x93 + x8))
                   : (e0 * x133 *
                      (-16 * eg * x156 * x37 + x1 * x122 * x31 * x33 * x96 +
                       x132 - x158 * x93)));
        const double d_c_imag =
            ((x28) ? (a * e0 * x90 * (-en * x157 * x89 + x32)) : (0));
        const cmpl d_c = cmpl(d_c_real, -d_c_imag);

        if (fb->form == TAUC_LORENTZ_STANDARD) {
            pd->at(osc_index + TL_AL_OFFS) = d_a;
            pd->at(osc_index + TL_E0_OFFS) = d_e0;
            pd->at(osc_index + TL_C_OFFS) = d_c;
        } else {
            const double a_p = fb->osc[k].a, e0_p = fb->osc[k].b,
                         c_p = fb->osc[k].c;
            const auto x0 = std::pow(c_p, 4);
            const auto x1 = std::pow(e0_p, 2);
            const auto x2 = 4 * std::pow(e0_p, 4) + x0;
            const auto x3 = std::sqrt(x2);
            const auto x4 = -2 * x1 + x3;
            const auto x5 = std::sqrt(x4);
            const auto x6 = 1.0 / x5;
            const auto x7 = M_SQRT2;
            const auto x8 = (1.0 / 4.0) * x7;
            const auto x9 = a_p / std::pow(x4, 3.0 / 2.0);
            const auto x10 = std::pow(c_p, 3);
            const auto x11 = x7 / std::pow(x2, 3.0 / 4.0);
            const auto x12 = 1.0 / x3;
            const double dp11 = x0 * x6 * x8 / std::pow(x2, 1.0 / 4.0);
            const double dp12 = (1.0 / 2.0) * e0_p * x0 * x7 * x9 *
                                (x1 * (4 * x1 - 2 * x3) + x3 * x4) /
                                std::pow(x2, 5.0 / 4.0);
            const double dp13 = x10 * x8 * x9 *
                                (-x0 * std::pow(x2, 3.0 / 2.0) - x0 * x2 * x4 +
                                 4 * std::pow(x2, 2) * x4) /
                                std::pow(x2, 9.0 / 4.0);
            const double dp21 = 0;
            const double dp22 = 2 * std::pow(e0_p, 3) * x11;
            const double dp23 = (1.0 / 2.0) * x10 * x11;
            const double dp31 = 0;
            const double dp32 = -2 * e0_p * x12 * x5;
            const double dp33 = x10 * x12 * x6;
            pd->at(osc_index + TL_AL_OFFS) =
                d_a * dp11 + d_e0 * dp21 + d_c * dp31;
            pd->at(osc_index + TL_E0_OFFS) =
                d_a * dp12 + d_e0 * dp22 + d_c * dp32;
            pd->at(osc_index + TL_C_OFFS) =
                d_a * dp13 + d_e0 * dp23 + d_c * dp33;
        }
    }

    cmpl nn = sqrt(cmpl(er_sum, -ei_sum));

    for (int i = 0; i < TL_NB_GLOBAL_PARAMS + nb * TL_NB_OSC_PARAMS; i++) {
        pd->at(i) = cmpl(1, 0) / (2 * nn) * pd->at(i);
    }

    return nn;
}
