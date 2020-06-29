/* Generated with sympy using the command: kramers-diff-sympy.py ../src/kramers-diff-template.cpp */
cmpl kramers_n_value_deriv(const disp_t *d, double lambda, cmpl_vector *pd) {
    const disp_kramers *kramers = &d->disp.kramers;
    const double e = wavelength_nm_to_energy_ev(lambda);

    double er_sum = 1.0, ei_sum = 0.0;
    for (int osc_index = KRAMERS_OSC_OFFS, k = 0; k < kramers->n; k++, osc_index += KRAMERS_NB_OSC_PARAMS) {
        const kramers_params *k_oscillator = &kramers->oscillators[k];
        const double a = k_oscillator->a, en = k_oscillator->en, eg = k_oscillator->eg, phi = k_oscillator->phi;

const auto x0 = std::cos(phi);
const auto x1 = std::pow(e, 2);
const auto x2 = std::pow(eg, 2)*x1;
const auto x3 = std::pow(en, 2);
const auto x4 = x1 - x3;
const auto x5 = std::pow(x4, 2);
const auto x6 = x2 + x5;
const auto x7 = 1.0/x6;
const auto x8 = x4*x7;
const auto x9 = x0*x8;
const auto x10 = std::sin(phi);
const auto x11 = eg*x10;
const auto x12 = std::pow(-x1 + x3, 2);
const auto x13 = x12 + x2;
const auto x14 = std::pow(x13, -2);
const auto x15 = x14*(-x12 + x2);
const auto x16 = x11*x15;
const auto x17 = en*x16 - x9;
const auto x18 = 2*eg;
const auto x19 = x10*x8;
const auto x20 = en*x19;
const auto x21 = e*x7;
const auto x22 = eg*x21;
const auto x23 = x22*(x0 + x18*x20);
const auto x24 = 2*x7;
const auto x25 = en*x0;
const auto x26 = std::pow(x6, -2);
const auto x27 = 4*x26;
const auto x28 = x2*x24;
const auto x29 = x11*x4;
const auto x30 = 2*x4;
const auto x31 = x1*x26;
const auto x32 = en*x10;
const auto x33 = 2/x13;
const auto x34 = x11*x3;

        er_sum += a*x17;
        ei_sum += a*x23;

        const double d_a_real = x17;
        const double d_a_imag = x23;
        const cmpl d_a = cmpl(d_a_real, -d_a_imag);
        pd->at(osc_index + KRAMERS_A_OFFS) += d_a;

        const double d_en_real = a*(x16 + x24*x25 - x25*x27*x5 - x27*x29*x3*(x24*x5 - x28 - 1));
        const double d_en_imag = a*e*x18*x26*(x25*x30 + x29 + 8*x34*x5*x7 - 2*x34);
        const cmpl d_en = cmpl(d_en_real, -d_en_imag);
        pd->at(osc_index + KRAMERS_EN_OFFS) += d_en;

        const double d_eg_real = a*(eg*x0*x30*x31 + 2*x14*x2*x32*(x12*x33 - x2*x33 + 1) + x15*x32);
        const double d_eg_imag = a*x21*(-8*std::pow(eg, 3)*x31*x32*x4 + 4*eg*x20 - x0*x28 + x0);
        const cmpl d_eg = cmpl(d_eg_real, -d_eg_imag);
        pd->at(osc_index + KRAMERS_EG_OFFS) += d_eg;

        const double d_phi_real = a*(eg*x15*x25 + x19);
        const double d_phi_imag = a*x22*(en*x18*x9 - x10);
        const cmpl d_phi = cmpl(d_phi_real, -d_phi_imag);
        pd->at(osc_index + KRAMERS_PHI_OFFS) += d_phi;
    }

    cmpl nn = sqrt(cmpl(er_sum, -ei_sum));

    for (int i = 0; i < kramers->n * KRAMERS_NB_OSC_PARAMS; i++) {
        pd->at(i) = cmpl(1, 0) / (2 * nn) * pd->at(i);
    }

    return nn;
}

