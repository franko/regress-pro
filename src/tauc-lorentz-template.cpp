cmpl tauc_lorentz_n_value_deriv(const disp_t *d, double lambda, cmpl_vector *pd) {
    const struct disp_fb *fb = &d->disp.fb;
    const int nb = fb->n;
    const double en = TL_EV_NM / lambda;

    /* If Eg is negative use zero instead. A negative Eg is not meaningful. */
    const double eg = (fb->eg >= 0 ? fb->eg : 0.0);

    pd->at(TL_NINF_OFFS) = cmpl(1.0, 0.0);
    pd->at(TL_EG_OFFS) = 0;

    double er_sum = fb->n_inf, ei_sum = 0.0;
    for (int osc_index = TL_OSC_OFFS, k = 0; k < nb; k++, osc_index += TL_NB_OSC_PARAMS) {
        double a, e0, c;
        oscillator_parameters(fb->form, fb->osc + k, a, e0, c);

        ${definitions}

        er_sum += ${eps1};
        ei_sum += ${eps2};

        const double d_eg_real = ${der_eps1_eg};
        const double d_eg_imag = ${der_eps2_eg};
        pd->at(TL_EG_OFFS) += cmpl(d_eg_real, -d_eg_imag);

        const double d_a_real = ${der_eps1_a};
        const double d_a_imag = ${der_eps2_a};
        pd->at(osc_index + TL_AL_OFFS) = cmpl(d_a_real, -d_a_imag);

        const double d_e0_real = ${der_eps1_e0};
        const double d_e0_imag = ${der_eps2_e0};
        pd->at(osc_index + TL_E0_OFFS) = cmpl(d_e0_real, -d_e0_imag);

        const double d_c_real = ${der_eps1_c};
        const double d_c_imag = ${der_eps2_c};
        pd->at(osc_index + TL_C_OFFS)  = cmpl(d_c_real, -d_c_imag);
    }

    cmpl nn = sqrt(cmpl(er_sum, -ei_sum));

    for (int i = 0; i < TL_NB_GLOBAL_PARAMS + nb * TL_NB_OSC_PARAMS; i++) {
        pd->at(i) = cmpl(1, 0) / (2 * nn) * pd->at(i);
    }

    return nn;
}
