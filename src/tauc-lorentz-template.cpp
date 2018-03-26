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
        const cmpl d_eg = cmpl(d_eg_real, -d_eg_imag);
        pd->at(TL_EG_OFFS) += d_eg;

        const double d_a_real = ${der_eps1_a};
        const double d_a_imag = ${der_eps2_a};
        const cmpl d_a = cmpl(d_a_real, -d_a_imag);

        const double d_e0_real = ${der_eps1_e0};
        const double d_e0_imag = ${der_eps2_e0};
        const cmpl d_e0 = cmpl(d_e0_real, -d_e0_imag);

        const double d_c_real = ${der_eps1_c};
        const double d_c_imag = ${der_eps2_c};
        const cmpl d_c = cmpl(d_c_real, -d_c_imag);

        if (fb->form == TAUC_LORENTZ_STANDARD) {
            pd->at(osc_index + TL_AL_OFFS) = d_a;
            pd->at(osc_index + TL_E0_OFFS) = d_e0;
            pd->at(osc_index + TL_C_OFFS ) = d_c;
        } else {
            const double a_p = fb->osc[k].a, e0_p = fb->osc[k].b, c_p = fb->osc[k].c;
const auto x0 = std::sqrt(2);
const auto x1 = std::pow(c_p, 4);
const auto x2 = 4*std::pow(e0_p, 4) + x1;
const auto x3 = std::pow(e0_p, 2);
const auto x4 = std::sqrt(x2);
const auto x5 = -2*x3 + x4;
const auto x6 = std::sqrt(x5);
const auto x7 = 1.0/x6;
const auto x8 = a_p/std::pow(x5, 3.0L/2.0L);
const auto x9 = std::pow(c_p, 3);
const auto x10 = x0/std::pow(x2, 3.0L/4.0L);
const auto x11 = 1.0/x4;
const double dp11 = (1.0L/4.0L)*x0*x1*x7/std::pow(x2, 1.0L/4.0L);
const double dp12 = (1.0L/2.0L)*e0_p*x0*x1*x8*(x3*(4*x3 - 2*x4) + x4*x5)/std::pow(x2, 5.0L/4.0L);
const double dp13 = (1.0L/4.0L)*x0*x8*x9*(-x1*std::pow(x2, 3.0L/2.0L) - x1*x2*x5 + 4*std::pow(x2, 2)*x5)/std::pow(x2, 9.0L/4.0L);
const double dp21 = 0;
const double dp22 = 2*std::pow(e0_p, 3)*x10;
const double dp23 = (1.0L/2.0L)*x10*x9;
const double dp31 = 0;
const double dp32 = -2*e0_p*x11*x6;
const double dp33 = x11*x7*x9;
            pd->at(osc_index + TL_AL_OFFS) = d_a * dp11 + d_e0 * dp21 + d_c * dp31;
            pd->at(osc_index + TL_E0_OFFS) = d_a * dp12 + d_e0 * dp22 + d_c * dp32;
            pd->at(osc_index + TL_C_OFFS ) = d_a * dp13 + d_e0 * dp23 + d_c * dp33;
        }
    }

    cmpl nn = sqrt(cmpl(er_sum, -ei_sum));

    for (int i = 0; i < TL_NB_GLOBAL_PARAMS + nb * TL_NB_OSC_PARAMS; i++) {
        pd->at(i) = cmpl(1, 0) / (2 * nn) * pd->at(i);
    }

    return nn;
}
