/* Generated with sympy using the command: kramers-sympy.py
 * ../src/kramers-template.cpp */
cmpl kramers_n_value(const disp_t *d, double lambda) {
    const disp_kramers *kramers = &d->disp.kramers;
    const double e = wavelength_nm_to_energy_ev(lambda);

    double er_sum = 1.0, ei_sum = 0.0;
    for (int k = 0; k < kramers->n; k++) {
        const kramers_params *k_oscillator = &kramers->oscillators[k];
        const double a = k_oscillator->a, en = k_oscillator->en,
                     eg = k_oscillator->eg, phi = k_oscillator->phi;

        /* Here we provided the variables a, en, eg, phi and e. These variables
           are used by the sympy automatic code. */

        const auto x0 = std::pow(e, 2);
        const auto x1 = std::pow(en, 2);
        const auto x2 = x0 - x1;
        const auto x3 = std::pow(x2, 2);
        const auto x4 = (x0 + x1) * std::sin(phi) / (x0 * x1 + x3);
        const auto x5 = x2 * x4;
        const auto x6 = -x5 + 2 * std::cos(phi);
        const auto x7 = (1.0 / 2.0) * a / (std::pow(eg, 2) * x0 + x3);

        double eps1 = -x7 * (eg * en * x0 * x4 + x2 * x6);
        double eps2 = e * x7 * (-eg * x6 + en * x5);
        er_sum += eps1;
        ei_sum += eps2;
    }
    return std::sqrt(er_sum - 1i * ei_sum);
}
