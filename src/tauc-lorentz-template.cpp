cmpl
tauc_lorentz_n_value(const disp_t *d, double lambda)
{
    const struct disp_fb *fb = &d->disp.fb;
    const int nb = fb->n;
    const double en = TL_EV_NM / lambda;
    const double eg_delta = 1.0e-4;

    /* If Eg is negative use zero instead. A negative Eg is not meaningful. */
    const double eg = (fb->eg >= 0 ? fb->eg : 0.0);
    double er_sum = fb->n_inf, ei_sum = 0.0;
    for (int k = 0; k < nb; k++) {
        double a, e0, c;
        oscillator_parameters(fb->form, fb->osc + k, a, e0, c);

${epsilon_defs}

        double eps1 = ${eps1};
        double eps2 = ${eps2};
        er_sum += eps1;
        ei_sum += eps2;
    }
    return std::sqrt(er_sum - 1i * ei_sum);
}
