
/* disp-tauc-lorentz.c
 *
 * Copyright (C) 2005-2015 Francesco Abbate
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <assert.h>
#include <string.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_deriv.h>

#include "cmpl.h"
#include "math-constants.h"
#include "dispers.h"
#include "fit-params.h"
#include "disp-fb-priv.h"

extern cmpl tauc_lorentz_n_value(const disp_t *disp, double lam);
static cmpl tauc_lorentz_n_value_deriv(const disp_t *disp, double lam,
                             cmpl_vector *der);
static void tauc_lorentz_encode_param(str_t param, const fit_param_t *fp);

using namespace std::complex_literals;

struct disp_class tauc_lorentz_disp_class = {
    DISP_TAUC_LORENTZ, "Tauc-Lorentz", "tauc-lorentz",
    fb_free,
    fb_copy,
    tauc_lorentz_n_value,
    fb_fp_number,
    tauc_lorentz_n_value_deriv,
    fb_apply_param,
    fb_map_param,
    fb_get_param_value,
    nullptr, nullptr,
    tauc_lorentz_encode_param,
    fb_read,
    fb_write,
};

static const char *tauc_lorentz_param_names[] = {"AL", "E0", "C"};

#define TL_EV_NM 1240.0

#define TL_NB_GLOBAL_PARAMS 2
#define TL_NINF_OFFS 0
#define TL_EG_OFFS 1
#define TL_OSC_OFFS 2
#define TL_NB_OSC_PARAMS 3
#define TL_AL_OFFS  0
#define TL_E0_OFFS  1
#define TL_C_OFFS  2

#define TL_PARAM_NB(hn,pn) (TL_NB_OSC_PARAMS * (hn) + pn)

static inline double pow2(double x) { return x * x; }

static inline double pow4(double x)
{
    const double s = x * x;
    return s * s;
}

static void oscillator_parameters(const int form, const fb_osc osc[], double& a, double& e0, double& c) {
    if (form == TAUC_LORENTZ_STANDARD) {
        a = osc->a;
        e0 = osc->b;
        c = osc->c;
    } else {
        /* Use rationalized lorentzian coefficients of the abs peak:
           - osc->a is AL' and gives the height of the peak
           - osc->b is Ep and is the energy position
           - osc->c is Gamma and is the peak width. */
        e0 = pow(pow4(osc->b) + pow4(osc->c) / 4, 0.25);
        c = sqrt(2 * pow2(e0) - 2 * pow2(osc->b));
        a = osc->a * pow4(osc->c) / (4 * e0 * c);
    }
}

cmpl
tauc_lorentz_n_value(const disp_t *d, double lambda)
{
    const struct disp_fb *fb = &d->disp.fb;
    const int nb = fb->n;
    const double en = TL_EV_NM / lambda;

    /* If Eg is negative use zero instead. A negative Eg is not meaningful. */
    const double eg = (fb->eg >= 0 ? fb->eg : 0.0);
    double er_sum = fb->n_inf, ei_sum = 0.0;
    for (int k = 0; k < nb; k++) {
        double a, e0, c;
        oscillator_parameters(fb->form, fb->osc + k, a, e0, c);

        double x0 = 1.0/M_PI;
        double x1 = std::pow(c, 2);
        double x2 = -x1;
        double x3 = std::pow(e0, 2);
        double x4 = x2 + 4*x3;
        double x5 = std::pow(en, 2);
        double x6 = -x3;
        double x7 = x5 + x6;
        double x8 = (1.0L/2.0L)*x1 + x7;
        double x9 = 1.0/((1.0L/4.0L)*x1*x4 + std::pow(x8, 2));
        double x10 = a*x0*x9/e0;
        double x11 = std::sqrt(x4);
        double x12 = 1.0/x11;
        double x13 = std::pow(eg, 2);
        double x14 = x1*x13;
        double x15 = x13 + x3;
        double x16 = eg*x11;
        double x17 = 1.0/c;
        double x18 = 2*eg;
        double x19 = 2*a*e0*eg*x0*x9;
        double x20 = 1.0/en;
        double x21 = eg + en;
        double x22 = fabs(eg - en);

        double eps1_1 = (1.0L/2.0L)*c*x10*x12*(x14 - x3*(3*x13 + x3) + x5*(x13 + x6))*log((x15 + x16)/(x15 - x16));
        double eps1_2 = -x10*(x14 + x15*x7)*(atan(x17*(x11 - x18)) - atan(x17*(x11 + x18)) + M_PI);
        double eps1_3 = x12*x19*x8*(2*atan(x12*x17*(-2*x13 + x2 + 2*x3)) + M_PI);
        double eps1_4 = -a*c*e0*x0*x20*x9*(x13 + x5)*log(x22/x21);
        double eps1_5 = c*x19*log(x21*x22/std::sqrt(x14 + std::pow(-x13 + x3, 2)));
        double eps2 = ((en > eg) ? a*c*e0*x20*std::pow(-eg + en, 2)/(x1*x5 + std::pow(x7, 2)) : 0);

        er_sum += eps1_1 + eps1_2 + eps1_3 + eps1_4 + eps1_5;
        ei_sum += eps2;
    }
    return std::sqrt(er_sum - 1i * ei_sum);
}

cmpl tauc_lorentz_n_value_deriv(const disp_t *d, double lambda, cmpl_vector *pd) {
    const struct disp_fb *fb = &d->disp.fb;
    const int nb = fb->n;
    double en = TL_EV_NM / lambda;

    /* If Eg is negative use zero instead. A negative Eg is not meaningful. */
    const double eg = (fb->eg >= 0 ? fb->eg : 0.0);

    pd->at(TL_NINF_OFFS) = cmpl(1.0, 0.0);
    pd->at(TL_EG_OFFS) = 0;

    double er_sum = fb->n_inf, ei_sum = 0.0;
    for (int osc_index = TL_OSC_OFFS, k = 0; k < nb; k++, osc_index += TL_NB_OSC_PARAMS) {
        double a, e0, c;
        oscillator_parameters(fb->form, fb->osc + k, a, e0, c);

const auto x0 = std::pow(eg, 2);
const auto x1 = std::pow(en, 2);
const auto x2 = x0 + x1;
const auto x3 = 1.0/M_PI;
const auto x4 = 1.0/en;
const auto x5 = eg + en;
const auto x6 = log(x5);
const auto x7 = -eg + en;
const auto x8 = en > eg;
const auto x9 = eg - en;
const auto x10 = ((x8) ? (
   log(x7)
)
: (
   log(x9)
));
const auto x11 = x10 - x6;
const auto x12 = std::pow(c, 2);
const auto x13 = -x12;
const auto x14 = std::pow(e0, 2);
const auto x15 = x13 + 4*x14;
const auto x16 = -x14;
const auto x17 = x1 + x16;
const auto x18 = (1.0L/2.0L)*x12 + x17;
const auto x19 = (1.0L/4.0L)*x12*x15 + std::pow(x18, 2);
const auto x20 = 1.0/x19;
const auto x21 = a*c*x11*x20*x3*x4;
const auto x22 = x2*x21;
const auto x23 = 2*e0*eg;
const auto x24 = x0*x12;
const auto x25 = -x0 + x14;
const auto x26 = x24 + std::pow(x25, 2);
const auto x27 = x10 + x6 - log(std::sqrt(x26));
const auto x28 = a*c*x20*x27*x3;
const auto x29 = std::sqrt(x15);
const auto x30 = 1.0/x29;
const auto x31 = 1.0/c;
const auto x32 = -2*x0 + x13 + 2*x14;
const auto x33 = x30*x32;
const auto x34 = 2*atan(x31*x33) + M_PI;
const auto x35 = a*x18*x20*x3*x30*x34;
const auto x36 = x0 + x14;
const auto x37 = x17*x36 + x24;
const auto x38 = 1.0/e0;
const auto x39 = 2*eg;
const auto x40 = x29 - x39;
const auto x41 = x29 + x39;
const auto x42 = atan(x31*x40) - atan(x31*x41) + M_PI;
const auto x43 = a*x20*x3*x38*x42;
const auto x44 = x0 + x16;
const auto x45 = 3*x0 + x14;
const auto x46 = x1*x44 - x14*x45 + x24;
const auto x47 = eg*x29;
const auto x48 = x36 + x47;
const auto x49 = x36 - x47;
const auto x50 = 1.0/x49;
const auto x51 = log(x48*x50);
const auto x52 = (1.0L/2.0L)*a*c*x20*x3*x30*x38*x51;
const auto x53 = std::pow(x7, 2);
const auto x54 = x1*x12 + std::pow(x17, 2);
const auto x55 = 1.0/x54;
const auto x56 = a*x4*x53*x55;
const auto x57 = c*x56;
const auto x58 = a*e0*x20*x3;
const auto x59 = 1.0/x5;
const auto x60 = ((x8) ? (
   -1/x7
)
: (
   1.0/x9
));
const auto x61 = 2*e0;
const auto x62 = 2*a*e0*x20*x3*x34;
const auto x63 = x12*x39;
const auto x64 = 1.0/x26;
const auto x65 = 1.0/x15;
const auto x66 = 1.0/x12;
const auto x67 = 1.0/(std::pow(x32, 2)*x65*x66 + 1);
const auto x68 = 2*x31;
const auto x69 = 1.0/(std::pow(x40, 2)*x66 + 1);
const auto x70 = 1.0/(std::pow(x41, 2)*x66 + 1);
const auto x71 = a*x20*x3*x37*x38;
const auto x72 = x48/std::pow(x49, 2);
const auto x73 = (1.0L/2.0L)*a*c*x20*x3*x30*x38*x46*x49/x48;
const auto x74 = x20*x3;
const auto x75 = 2*e0*eg*x18*x30*x34;
const auto x76 = x37*x38*x42;
const auto x77 = (1.0L/2.0L)*c*x30*x38*x46*x51;
const auto x78 = 4*eg*x14;
const auto x79 = 4*e0*x18 - x12*x61;
const auto x80 = std::pow(x19, -2);
const auto x81 = a*c*e0*x11*x2*x3*x4*x80;
const auto x82 = std::pow(x15, -3.0L/2.0L);
const auto x83 = a*x20*x3*x34*x82;
const auto x84 = a*x20*x3/x14;
const auto x85 = 2*a*c*e0*eg*x27*x3*x80;
const auto x86 = a*x3*x79*x80;
const auto x87 = x46*x51*x82;
const auto x88 = 4*e0*x30;
const auto x89 = x31*x88;
const auto x90 = x32*x82;
const auto x91 = 4*a*e0*eg*x18*x20*x3*x30*x67;
const auto x92 = eg*x88;
const auto x93 = c*eg*x30;
const auto x94 = a*x20*x3*x30*x38*x51;
const auto x95 = (1.0L/2.0L)*std::pow(c, 3) - 1.0L/2.0L*c*x15 - 2*c*x18;
const auto x96 = a*x3*x80*x95;
const auto x97 = -x30;
const auto x98 = c*e0*x4*x55;
const auto x99 = std::pow(x54, -2);

        er_sum += -e0*x22 + x23*x28 + x23*x35 - x37*x43 + x46*x52;
        ei_sum += ((x8) ? (
   e0*x57
)
: (
   0
));

        const double d_eg_real = 2*a*c*e0*eg*x20*x3*(x59 + x60 - x64*(eg*x12 - x25*x39)) - c*x2*x4*x58*(-x59 + x60) - 16*x0*x18*x31*x58*x65*x67 + x18*x30*x62 - x21*x23 + x28*x61 - x43*(x17*x39 + x63) + x52*(-6*eg*x14 + x1*x39 + x63) - x71*(-x68*x69 - x68*x70) + x73*(x40*x72 + x41*x50);
        const double d_eg_imag = ((x8) ? (
   a*x98*(-2*en + x39)
)
: (
   0
));
        pd->at(TL_EG_OFFS) += cmpl(d_eg_real, -d_eg_imag);

        const double d_a_real = 2*c*e0*eg*x27*x74 - c*e0*x11*x2*x4*x74 + x74*x75 - x74*x76 + x74*x77;
        const double d_a_imag = ((x8) ? (
   x53*x98
)
: (
   0
));
        pd->at(osc_index + TL_AL_OFFS) = cmpl(d_a_real, -d_a_imag);

        const double d_e0_real = -a*c*x20*x25*x3*x64*x78 - 2*a*c*x20*x3*x87 - a*x20*x3*x30*x34*x78 - 1.0L/2.0L*c*x30*x46*x51*x84 - 8*eg*x14*x18*x83 - x22 + x28*x39 + x35*x39 + x37*x42*x84 - x43*(x17*x61 - x36*x61) + x52*(-2*std::pow(e0, 3) - x1*x61 - x45*x61) - x71*(x69*x89 - x70*x89) + x73*(x50*(x61 + x92) + x72*(-x61 + x92)) + x75*x86 - x76*x86 + x77*x86 - x79*x81 + x79*x85 + x91*(-4*e0*x31*x90 + x89);
        const double d_e0_imag = ((x8) ? (
   4*a*c*x14*x17*x4*x53*x99 + x57
)
: (
   0
));
        pd->at(osc_index + TL_E0_OFFS) = cmpl(d_e0_real, -d_e0_imag);

        const double d_c_real = -2*a*e0*std::pow(eg, 3)*x12*x20*x3*x64 + 2*a*e0*eg*x20*x27*x3 - a*e0*x11*x2*x20*x3*x4 + (1.0L/2.0L)*a*x12*x20*x3*x38*x87 + 2*c*e0*eg*x18*x83 - 2*c*x0*x43 + x24*x94 + x62*x93 - x71*(x69*(-x40*x66 + x97) - x70*(-x41*x66 + x97)) + x73*(-x50*x93 - x72*x93) + x75*x96 - x76*x96 + x77*x96 - x81*x95 + x85*x95 + x91*(-2*x30 - x33*x66 + x90) + x94*((1.0L/2.0L)*x0*x12 + (1.0L/2.0L)*x1*x44 - 1.0L/2.0L*x14*x45);
        const double d_c_imag = ((x8) ? (
   -2*a*e0*en*x12*x53*x99 + e0*x56
)
: (
   0
));
        pd->at(osc_index + TL_C_OFFS)  = cmpl(d_c_real, -d_c_imag);
    }

    cmpl nn = sqrt(cmpl(er_sum, -ei_sum));

    for (int i = 0; i < TL_NB_GLOBAL_PARAMS + nb * TL_NB_OSC_PARAMS; i++) {
        pd->at(i) = cmpl(1, 0) / (2 * nn) * pd->at(i);
    }

    return nn;
}

void tauc_lorentz_change_form(struct disp_fb *fb, int new_coeff_form)
{
    if (new_coeff_form == TAUC_LORENTZ_STANDARD) {
        int i;
        for (i = 0; i < fb->n; i++) {
            struct fb_osc *osc = &fb->osc[i];
            const double ALp = osc->a, Ep = osc->b, G = osc->c;
            const double Gqq = pow4(G);
            const double E0 = pow(pow4(Ep) + Gqq / 4, 0.25);
            const double C = sqrt(2 * pow2(E0) - 2 * pow2(Ep));
            const double AL = ALp * Gqq / (4 * E0 * C);
            osc->a = AL;
            osc->b = E0;
            osc->c = C;
        }
    } else {
        int i;
        for (i = 0; i < fb->n; i++) {
            struct fb_osc *osc = &fb->osc[i];
            const double AL = osc->a, E0 = osc->b, C = osc->c;
            const double Cq = pow2(C);
            const double Gqq = (4 * pow2(E0) - Cq) * Cq;
            const double G = pow(Gqq, 0.25);
            const double Ep = sqrt(pow2(E0) - Cq / 2);
            const double ALp = (4 * AL * E0 * C) / Gqq;
            osc->a = ALp;
            osc->b = Ep;
            osc->c = G;
        }
    }
}

void
tauc_lorentz_encode_param(str_t param, const fit_param_t *fp)
{
    if (fp->param_nb == 0) {
        str_copy_c(param, "EpsInf");
    } else if (fp->param_nb == 1) {
        str_copy_c(param, "Eg");
    } else {
        int onb = (fp->param_nb - TL_NB_GLOBAL_PARAMS) / TL_NB_OSC_PARAMS;
        int pnb = (fp->param_nb - TL_NB_GLOBAL_PARAMS) % TL_NB_OSC_PARAMS;
        str_printf(param, "%s:%i", tauc_lorentz_param_names[pnb], onb);
    }
}
