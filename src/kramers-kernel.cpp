/* Generated with sympy using the command: kramers-sympy.py ../src/kramers-template.cpp */
cmpl kramers_n_value(const disp_t *d, double lambda) {
    const disp_kramers *kramers = &d->disp.kramers;
    const double e = wavelength_nm_to_energy_ev(lambda);

    double er_sum = 1.0, ei_sum = 0.0;
    for (int k = 0; k < kramers->n; k++) {
        const kramers_params *k_oscillator = &kramers->oscillators[k];
        const double a = k_oscillator->a, en = k_oscillator->en, eg = k_oscillator->eg, phi = k_oscillator->phi;

        /* Here we provided the variables a, en, eg, phi and e. These variables
           are used by the sympy automatic code. */

const auto x0 = std::cos(phi);
const auto x1 = std::pow(e, 2);
const auto x2 = std::pow(eg, 2)*x1;
const auto x3 = std::pow(en, 2);
const auto x4 = x1 - x3;
const auto x5 = 1.0/(x2 + std::pow(x4, 2));
const auto x6 = x4*x5;
const auto x7 = std::pow(-x1 + x3, 2);
const auto x8 = eg*en*std::sin(phi);

        double eps1 = a*(-x0*x6 + x8*(x2 - x7)/std::pow(x2 + x7, 2));
        double eps2 = a*e*eg*x5*(x0 + 2*x6*x8);
        er_sum += eps1;
        ei_sum += eps2;
    }
    return std::sqrt(er_sum - 1i * ei_sum);
}

