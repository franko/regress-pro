/* Generated with sympy using the command: kramers-diff-sympy.py
 * ../src/kramers-diff-template.cpp */
cmpl kramers_n_value_deriv(const disp_t *d, double lambda, cmpl_vector *pd) {
    const disp_kramers *kramers = &d->disp.kramers;
    const double e = wavelength_nm_to_energy_ev(lambda);

    double er_sum = 1.0, ei_sum = 0.0;
    for (int osc_index = KRAMERS_OSC_OFFS, k = 0; k < kramers->n;
         k++, osc_index += KRAMERS_NB_OSC_PARAMS) {
        const kramers_params *k_oscillator = &kramers->oscillators[k];
        const double a = k_oscillator->a, en = k_oscillator->en,
                     eg = k_oscillator->eg, phi = k_oscillator->phi;

        const auto x0 = std::sin(phi);
        const auto x1 = std::pow(e, 2);
        const auto x2 = std::pow(en, 2);
        const auto x3 = x1 * x2;
        const auto x4 = x1 - x2;
        const auto x5 = std::pow(x4, 2);
        const auto x6 = x3 + x5;
        const auto x7 = 1.0 / x6;
        const auto x8 = x0 * x7;
        const auto x9 = eg * x8;
        const auto x10 = en * x9;
        const auto x11 = x1 + x2;
        const auto x12 = x1 * x11;
        const auto x13 = std::cos(phi);
        const auto x14 = 2 * x13;
        const auto x15 = x11 * x4;
        const auto x16 = x15 * x8;
        const auto x17 = x14 - x16;
        const auto x18 = x17 * x4;
        const auto x19 = std::pow(eg, 2) * x1;
        const auto x20 = 1.0 / (x19 + x5);
        const auto x21 = (1.0 / 2.0) * x20;
        const auto x22 = x21 * (x10 * x12 + x18);
        const auto x23 = a * x21;
        const auto x24 = en * x16;
        const auto x25 = eg * (-x14 + x16);
        const auto x26 = e * (x24 + x25);
        const auto x27 = (1.0 / 2.0) * x11;
        const auto x28 = en * x17;
        const auto x29 = 2 * x2;
        const auto x30 = -x1 + x29;
        const auto x31 = x0 * x30 / std::pow(x6, 2);
        const auto x32 = 2 * x0;
        const auto x33 = x15 * x7;
        const auto x34 = eg * x20;
        const auto x35 = 2 * x20;
        const auto x36 = x29 + x30 * x33;
        const auto x37 = en * x8;
        const auto x38 = a * x20;
        const auto x39 = x19 * x20;
        const auto x40 = x13 * x33;
        const auto x41 = x32 + x40;
        const auto x42 = x2 * x8;
        const auto x43 = -1.0 / 2.0 * x16;
        const auto x44 = e * x38;

        er_sum += -a * x22;
        ei_sum += -x23 * x26;

        const double d_a_real = -x22;
        const double d_a_imag = -x21 * x26;
        const cmpl d_a = cmpl(d_a_real, -d_a_imag);
        pd->at(osc_index + KRAMERS_A_OFFS) += d_a;

        const double d_en_real =
            x38 * (eg * x11 * x3 * x31 - x1 * x27 * x9 - x28 * x35 * x5 + x28 -
                   x3 * x32 * x33 * x34 - x3 * x9 - x36 * x37 * x4);
        const double d_en_imag = x44 * (-en * x25 * x35 * x4 + x10 * x36 -
                                        x11 * x20 * x29 * x5 * x8 + x11 * x42 +
                                        x15 * x2 * x31 - x4 * x42 + x43);
        const cmpl d_en = cmpl(d_en_real, -d_en_imag);
        pd->at(osc_index + KRAMERS_EN_OFFS) += d_en;

        const double d_eg_real =
            x1 * x38 * (x11 * x37 * x39 + x18 * x34 - x27 * x37);
        const double d_eg_imag = x44 * (x1 * x24 * x34 + x13 - x17 * x39 + x43);
        const cmpl d_eg = cmpl(d_eg_real, -d_eg_imag);
        pd->at(osc_index + KRAMERS_EG_OFFS) += d_eg;

        const double d_phi_real = x23 * (-eg * en * x12 * x13 * x7 + x4 * x41);
        const double d_phi_imag = -e * x23 * (eg * x41 + en * x40);
        const cmpl d_phi = cmpl(d_phi_real, -d_phi_imag);
        pd->at(osc_index + KRAMERS_PHI_OFFS) += d_phi;
    }

    cmpl nn = sqrt(cmpl(er_sum, -ei_sum));

    for (int i = 0; i < kramers->n * KRAMERS_NB_OSC_PARAMS; i++) {
        pd->at(i) = cmpl(1, 0) / (2 * nn) * pd->at(i);
    }

    return nn;
}
