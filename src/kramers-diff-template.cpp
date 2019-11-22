cmpl kramers_n_value_deriv(const disp_t *d, double lambda, cmpl_vector *pd) {
    const disp_kramers *kramers = &d->disp.kramers;
    const double e = wavelength_nm_to_energy_ev(lambda);

    double er_sum = 1.0, ei_sum = 0.0;
    for (int osc_index = KRAMERS_OSC_OFFS, k = 0; k < kramers->n; k++, osc_index += KRAMERS_NB_OSC_PARAMS) {
        const kramers_params *k_oscillator = &kramers->oscillators[k];
        const double a = k_oscillator->a, en = k_oscillator->en, eg = k_oscillator->eg, phi = k_oscillator->phi;

${epsilon_defs}

        er_sum += ${eps1};
        ei_sum += ${eps2};

        const double d_a_real = ${der_eps1_a};
        const double d_a_imag = ${der_eps2_a};
        const cmpl d_a = cmpl(d_a_real, -d_a_imag);
        pd->at(osc_index + KRAMERS_A_OFFS) += d_a;

        const double d_en_real = ${der_eps1_en};
        const double d_en_imag = ${der_eps2_en};
        const cmpl d_en = cmpl(d_en_real, -d_en_imag);
        pd->at(osc_index + KRAMERS_EN_OFFS) += d_en;

        const double d_eg_real = ${der_eps1_eg};
        const double d_eg_imag = ${der_eps2_eg};
        const cmpl d_eg = cmpl(d_eg_real, -d_eg_imag);
        pd->at(osc_index + KRAMERS_EG_OFFS) += d_eg;

        const double d_phi_real = ${der_eps1_phi};
        const double d_phi_imag = ${der_eps2_phi};
        const cmpl d_phi = cmpl(d_phi_real, -d_phi_imag);
        pd->at(osc_index + KRAMERS_PHI_OFFS) += d_phi;
    }

    cmpl nn = sqrt(cmpl(er_sum, -ei_sum));

    for (int i = 0; i < kramers->n * KRAMERS_NB_OSC_PARAMS; i++) {
        pd->at(i) = cmpl(1, 0) / (2 * nn) * pd->at(i);
    }

    return nn;
}
