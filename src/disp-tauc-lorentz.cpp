
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

#include "tauc-lorentz-kernel.cpp"

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
