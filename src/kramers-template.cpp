cmpl kramers_n_value(const disp_t *d, double lambda) {
    const disp_kramers *kramers = &d->disp.kramers;
    const double e = wavelength_nm_to_energy_ev(lambda);

    double er_sum = 1.0, ei_sum = 0.0;
    for (int k = 0; k < kramers->n; k++) {
        const kramers_params *k_oscillator = &kramers->oscillators[k];
        const double a = k_oscillator->a, en = k_oscillator->en, eg = k_oscillator->eg, phi = k_oscillator->phi;

        /* Here we provided the variables a, en, eg, phi and e. These variables
           are used by the sympy automatic code. */

${epsilon_defs}

        double eps1 = ${eps1};
        double eps2 = ${eps2};
        er_sum += eps1;
        ei_sum += eps2;
    }
    return std::sqrt(er_sum - 1i * ei_sum);
}
