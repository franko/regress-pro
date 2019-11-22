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
        const auto x14 = x11 * x4;
        const auto x15 = x14 * x8;
        const auto x16 = 2 * x13 - x15;
        const auto x17 = x16 * x4;
        const auto x18 = std::pow(eg, 2) * x1;
        const auto x19 = 1.0 / (x18 + x5);
        const auto x20 = (1.0 / 2.0) * x19;
        const auto x21 = x20 * (x10 * x12 + x17);
        const auto x22 = a * x20;
        const auto x23 = en * x15;
        const auto x24 = e * (-eg * x16 + x23);
        const auto x25 = (1.0 / 2.0) * x11;
        const auto x26 = en * x16;
        const auto x27 = 2 * x2;
        const auto x28 = -x1 + x27;
        const auto x29 = x0 * x28 / std::pow(x6, 2);
        const auto x30 = 2 * x0;
        const auto x31 = x14 * x7;
        const auto x32 = eg * x19;
        const auto x33 = x19 * x5;
        const auto x34 = x27 + x28 * x31;
        const auto x35 = en * x8;
        const auto x36 = a * x19;
        const auto x37 = x18 * x19;
        const auto x38 = x17 * x32;
        const auto x39 = x13 * x31;
        const auto x40 = x30 + x39;
        const auto x41 = x2 * x8;
        const auto x42 = (1.0 / 2.0) * x15;
        const auto x43 = e * x36;

        er_sum += -a * x21;
        ei_sum += x22 * x24;

        const double d_a_real = -x21;
        const double d_a_imag = x20 * x24;
        const cmpl d_a = cmpl(d_a_real, -d_a_imag);
        pd->at(osc_index + KRAMERS_A_OFFS) += d_a;

        const double d_en_real =
            x36 * (eg * x11 * x29 * x3 - x1 * x25 * x9 - 2 * x26 * x33 + x26 -
                   x3 * x30 * x31 * x32 - x3 * x9 - x34 * x35 * x4);
        const double d_en_imag =
            x43 * (-2 * en * x38 - x10 * x34 + x11 * x27 * x33 * x8 -
                   x11 * x41 - x14 * x2 * x29 + x4 * x41 + x42);
        const cmpl d_en = cmpl(d_en_real, -d_en_imag);
        pd->at(osc_index + KRAMERS_EN_OFFS) += d_en;

        const double d_eg_real = x1 * x36 * (x11 * x35 * x37 - x25 * x35 + x38);
        const double d_eg_imag =
            x43 * (-x1 * x23 * x32 - x13 + x16 * x37 + x42);
        const cmpl d_eg = cmpl(d_eg_real, -d_eg_imag);
        pd->at(osc_index + KRAMERS_EG_OFFS) += d_eg;

        const double d_phi_real = x22 * (-eg * en * x12 * x13 * x7 + x4 * x40);
        const double d_phi_imag = e * x22 * (eg * x40 + en * x39);
        const cmpl d_phi = cmpl(d_phi_real, -d_phi_imag);
        pd->at(osc_index + KRAMERS_PHI_OFFS) += d_phi;
    }

    cmpl nn = sqrt(cmpl(er_sum, -ei_sum));

    for (int i = 0; i < kramers->n * KRAMERS_NB_OSC_PARAMS; i++) {
        pd->at(i) = cmpl(1, 0) / (2 * nn) * pd->at(i);
    }

    return nn;
}
