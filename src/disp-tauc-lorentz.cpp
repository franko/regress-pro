
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

#include "tauc-lorentz-kernel.cpp"
#include "tauc-lorentz-diff-kernel.cpp"

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
