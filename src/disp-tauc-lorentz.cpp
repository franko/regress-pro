
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

static cmpl tauc_lorentz_n_value(const disp_t *disp, double lam);
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
#define TL_NB_PARAMS 3
#define TL_AL_OFFS  0
#define TL_E0_OFFS  1
#define TL_C_OFFS  2

#define TL_PARAM_NB(hn,pn) (TL_NB_PARAMS * (hn) + pn)

static double egap_k(const double E, const double Eg, const double k)
{
    return (E > Eg ? k : 0.0);
}

static inline double pow2(double x) { return x * x; }

static inline double pow4(double x)
{
    const double s = x * x;
    return s * s;
}

cmpl
tauc_lorentz_n_value(const disp_t *d, double lambda)
{
    const struct disp_fb *fb = &d->disp.fb;
    int k, nb = fb->n;
    double er_sum = fb->n_inf, ei_sum = 0;
    double E = TL_EV_NM / lambda;

    /* If Eg is negative use zero instead. A negative Eg is not meaningful. */
    const double Eg = (fb->eg >= 0 ? fb->eg : 0.0);
    for(k = 0; k < nb; k++) {
        const struct fb_osc *osc = fb->osc + k;
        double A, E0, C;
        if (fb->form == TAUC_LORENTZ_STANDARD) {
            A = osc->a;
            E0 = osc->b;
            C = osc->c;
        } else {
            /* Use rationalized lorentzian coefficients of the abs peak:
               - osc->a is AL' and gives the height of the peak
               - osc->b is Ep and is the energy position
               - osc->c is Gamma and is the peak width. */
            E0 = pow(pow4(osc->b) + pow4(osc->c) / 4, 0.25);
            C = sqrt(2 * pow2(E0) - 2 * pow2(osc->b));
            A = osc->a * pow4(osc->c) / (4 * E0 * C);
        }
        const double Eq = pow2(E), Cq = pow2(C), E0q = pow2(E0), Egq = pow2(Eg);
        const double den = pow2(Eq - E0q) + Cq * Eq;

        int alpha_real = (C < (2 - 1e-10) * E0);

        const double a_ln = (Egq - E0q) * Eq + Egq * Cq - E0q * (E0q + 3 * Egq);
        const double a_tan = (Eq - E0q) * (E0q + Egq) + Egq * Cq;
        const double alpha_sq = (alpha_real ? 4 * E0q - Cq : 0.0);
        const double gamma_sq = E0q - Cq / 2;
        const double zeta4 = pow2(Eq - gamma_sq) + alpha_sq * Cq / 4;

        const double ei_term = (A * E0 * C * pow2(E - Eg)) / (E * den);
        ei_sum += egap_k(E, Eg, ei_term);

        const double pi_zeta4 = M_PI * zeta4;
        if (!alpha_real) {
            /* In this case alpha is close to 0 or it is imaginary. To avoid NaNs we consider
               alpha = 0 and use a special form of the expression. Note in this case that
               gamma^2 is negative equal to -E0^2. */
            const double er_term1 = (2 * Eg * A * C * a_ln) / (2 * pi_zeta4 * E0 * (E0q + Egq));
            const double er_term2 = - (A * a_tan) / (pi_zeta4 * E0) * (2 * atan(C / (2 * Eg)));
            const double er_term3 = (2 * A * E0 * Eg * (Eq - gamma_sq)) / pi_zeta4 * (C / (E0q + Egq));
            er_sum += er_term1 + er_term2 + er_term3;
        } else {
            const double alpha = sqrt(alpha_sq);
            const double atanp = atan((alpha + 2 * Eg) / C), atanm = atan((alpha - 2 * Eg) / C);
            const double er_term1 = (A * C * a_ln) / (2 * pi_zeta4 * alpha * E0) * log((E0q + Egq + alpha * Eg) / (E0q + Egq - alpha * Eg));
            const double er_term2 = - (A * a_tan) / (pi_zeta4 * E0) * (M_PI - atanp + atanm);
            const double er_term3 = (2 * A * E0 * Eg * (Eq - gamma_sq)) / (pi_zeta4 * alpha) * (M_PI + 2 * atan(2 * (gamma_sq - Egq) / (alpha * C)));
            er_sum += er_term1 + er_term2 + er_term3;
        }

        const double log_den = sqrt(pow2(E0q - Egq) + Egq * Cq);
        if (fabs(E - Eg) < 1e-10 * E) {
            /* If E is very close to Eg use an alternative form that avoid log(0). */
            er_sum += (2 * A * E * E0 * C) / pi_zeta4 * log((4 * Eq) / log_den);
        } else {
            const double er_term4 = - (A * E0 * C * (Eq + Egq)) / (pi_zeta4 * E) * log(fabs(E - Eg)/(E + Eg));
            const double er_term5 = (2 * A * E0 * C * Eg) / pi_zeta4 * log((fabs(E - Eg) * (E + Eg)) / log_den);
            er_sum += er_term4 + er_term5;
        }
    }

    return std::sqrt(er_sum - 1i * ei_sum);
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

struct eval_param {
    disp_t *d;
    double lambda;
    double *param_ptr;
};

double n_eval_real(double x, void *params)
{
    struct eval_param *p = (struct eval_param *) params;
    *p->param_ptr = x;
    cmpl n = n_value(p->d, p->lambda);
    return std::real(n);
}

double n_eval_imag(double x, void *params)
{
    struct eval_param *p = (struct eval_param *) params;
    *p->param_ptr = x;
    cmpl n = n_value(p->d, p->lambda);
    return std::imag(n);
}

static void
tauc_lorentz_num_deriv(const disp_t *d, double lambda, cmpl_vector *pd)
{
    struct eval_param p[1];

    /* Explicitly discard the const qualifier. The dispersion object will
       change but before the end of the function it will be restored to its
       original state. */
    p->d = (disp_t *) d;
    p->lambda = lambda;

    fit_param_t fit_param[1];
    fit_param->id = PID_LAYER_N;
    fit_param->layer_nb = 0;
    fit_param->model_id = DISP_TAUC_LORENTZ;

    gsl_function F;
    F.params = (void *) p;
    int no_params = disp_get_number_of_params(p->d);
    int i;
    for (i = 0; i < no_params; i++) {
        fit_param->param_nb = i;
        p->param_ptr = disp_map_param(p->d, i);

        double rderiv, ideriv;

        F.function = &n_eval_real;
        double abserr;
        double p0 = *p->param_ptr;
        double h = p0 * 1e-4;
        h = (h < 1e-6 ? 1e-6 : h);
        gsl_deriv_central(&F, p0, h, &rderiv, &abserr);
        *p->param_ptr = p0;

        F.function = &n_eval_imag;
        gsl_deriv_central(&F, p0, h, &ideriv, &abserr);
        *p->param_ptr = p0;

        pd->at(i) = rderiv + 1i * ideriv;
    }
}

cmpl
tauc_lorentz_n_value_deriv(const disp_t *d, double lambda, cmpl_vector *pd)
{
    cmpl n = tauc_lorentz_n_value(d, lambda);
    if (pd) {
        tauc_lorentz_num_deriv(d, lambda, pd);
    }
    return n;
}

void
tauc_lorentz_encode_param(str_t param, const fit_param_t *fp)
{
    if (fp->param_nb == 0) {
        str_copy_c(param, "EpsInf");
    } else if (fp->param_nb == 1) {
        str_copy_c(param, "Eg");
    } else {
        int onb = (fp->param_nb - TL_NB_GLOBAL_PARAMS) / TL_NB_PARAMS;
        int pnb = (fp->param_nb - TL_NB_GLOBAL_PARAMS) % TL_NB_PARAMS;
        str_printf(param, "%s:%i", tauc_lorentz_param_names[pnb], onb);
    }
}
