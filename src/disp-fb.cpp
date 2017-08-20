
/* disp-fb.c
 *
 * Copyright (C) 2005-2011 Francesco Abbate
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
#include "dispers.h"
#include "cmpl.h"
#include "disp-fb-priv.h"

struct disp_class fb_disp_class = {
    DISP_FB, "Forouhi-Bloomer", "forouhi-bloomer",
    fb_free,
    fb_copy,
    fb_n_value,
    fb_fp_number,
    fb_n_value_deriv,
    fb_apply_param,
    fb_map_param,
    fb_get_param_value,
    nullptr, nullptr,
    fb_encode_param,
    fb_read,
    fb_write,
};

static const char *fb_param_names[] = {"A", "B", "C"};

#define FB_EV_NM 1240.0

#define FB_NB_GLOBAL_PARAMS 2
#define FB_NINF_OFFS 0
#define FB_EG_OFFS 1
#define FB_NB_PARAMS 3
#define FB_A_OFFS  0
#define FB_B_OFFS  1
#define FB_C_OFFS  2

#define FB_PARAM_NB(hn,pn) (FB_NB_PARAMS * (hn) + pn)

void
fb_free(disp_t *d)
{
    struct disp_fb *fb = & d->disp.fb;
    free(fb->osc);
    disp_base_free(d);
}

disp_t *
fb_copy(const disp_t *src)
{
    struct disp_struct *res;
    struct disp_fb *fb;
    struct fb_osc *osc;

    res = disp_base_copy(src);

    fb = & res->disp.fb;
    osc = fb->osc;
    fb->osc = (struct fb_osc *) emalloc(sizeof(struct fb_osc) * fb->n);
    memcpy(fb->osc, osc, sizeof(struct fb_osc) * fb->n);

    return res;
}

void
disp_add_osc(struct disp_struct *d)
{
    struct disp_fb *fb = &d->disp.fb;
    int n = fb->n;
    struct fb_osc *osc = (struct fb_osc *) emalloc(sizeof(struct fb_osc) * (n + 1));
    memcpy(osc, fb->osc, sizeof(struct fb_osc) * n);
    osc[n].a = 0.0;
    if (d->type == DISP_FB) {
        if (fb->form == FOROUHI_BLOOMER_STANDARD) {
            osc[n].b = 5.8;
            osc[n].c = 10;
        } else {
            osc[n].b = 2.9;
            osc[n].c = 1.260955;
        }
    } else {
        if (fb->form == TAUC_LORENTZ_STANDARD) {
            osc[n].b = 3.5;
            osc[n].c = 2;
        } else {
            osc[n].b = 3.20156;
            osc[n].c = 3.66284;
        }

    }

    free(fb->osc);
    fb->osc = osc;
    fb->n = n + 1;
}

void
disp_delete_osc(struct disp_struct *d, int index)
{
    struct disp_fb *fb = &d->disp.fb;
    int n = fb->n;
    int nrem = n - index - 1;
    memcpy(fb->osc + index, fb->osc + index + 1, sizeof(struct fb_osc) * nrem);
    fb->n = n - 1;
}

cmpl
fb_n_value(const disp_t *disp, double lam)
{
    return fb_n_value_deriv(disp, lam, nullptr);
}

static double egap_k(const double E, const double Eg, const double k)
{
    return (E > Eg ? k : 0.0);
}

cmpl
fb_n_value_deriv(const disp_t *d, double lambda, cmpl_vector *pd)
{
    const struct disp_fb *fb = &d->disp.fb;
    int k, nb = fb->n;
    double nsum = fb->n_inf, ksum = 0;
    double E = FB_EV_NM / lambda;

    const double Eg = fb->eg;
    cmpl dndeg = 0.0; /* Accumulate contribution to the derivative of complex n with Eg. */
    for(k = 0; k < nb; k++) {
        const struct fb_osc *osc = fb->osc + k;
        double A, B, C;
        if (fb->form == FOROUHI_BLOOMER_STANDARD) {
            A = osc->a;
            B = osc->b;
            C = osc->c;
        } else {
            /* Use the redefined A', B', C' parameters as described in the disp-fb.h file.
               The calculation are done based on the original parameters A, B, C. The
               derivatives are computed from the derivative wrt A, B, C. */
            A = osc->a * SQR(osc->c);
            B = 2 * osc->b;
            C = SQR(osc->c) + SQR(osc->b);
        }
        const double den = E * (E - B) + C;
        const double Q = 0.5 * sqrt(4*C - SQR(B));
        const double B0 = (A / Q) * (-SQR(B)/2 + Eg * (B - Eg) + C);
        const double C0 = (A / Q) * ((SQR(Eg) + C)*B/2 - 2 * Eg * C);

        const double kterm = A * SQR(E - Eg) / den;
        const double nterm = (B0*E + C0) / den;

        ksum += egap_k(E, Eg, kterm);
        nsum += nterm;

        if (pd) {
            const int koffs = FB_NB_GLOBAL_PARAMS + k * FB_NB_PARAMS;

            const double k_eg = egap_k(E, Eg, -2 * A * (E - Eg) / den);
            const double n_eg = (A / Q) * (E * B - 2*E*Eg + Eg * B - 2*C) / den;
            dndeg += cmpl(n_eg, -k_eg);

            /* Derivatives */
            const double k_a = egap_k(E, Eg, kterm / A);
            const double n_a = nterm / A;

            const double dB0dB = A * (B*SQR(B) + 8*C*Eg - 2*B*(3*C+SQR(Eg))) / (8 * Q*SQR(Q));
            const double dC0dB = A * C * (C + Eg * (Eg - B)) / (2 * Q*SQR(Q));

            const double k_b = egap_k(E, Eg, E * kterm / den);
            const double n_b = (dB0dB * E + dC0dB) / den + E * nterm / den;

            const double dB0dC = A * (C + Eg * (Eg - B)) / (2 * Q*SQR(Q));
            const double dC0dC = A * ((B - 4*Eg) * (2*C - SQR(B)) - 2*B*SQR(Eg)) / (8 * Q*SQR(Q));
            const double k_c = egap_k(E, Eg, - kterm / den);
            const double n_c = (dB0dC * E + dC0dC) / den - nterm / den;

            if (fb->form == FOROUHI_BLOOMER_STANDARD) {
                cmpl_vector_set(pd, koffs + FB_A_OFFS, cmpl(n_a, -k_a));
                cmpl_vector_set(pd, koffs + FB_B_OFFS, cmpl(n_b, -k_b));
                cmpl_vector_set(pd, koffs + FB_C_OFFS, cmpl(n_c, -k_c));
            } else {
                cmpl_vector_set(pd, koffs + FB_A_OFFS, SQR(osc->c) * (cmpl(n_a, -k_a)));
                cmpl_vector_set(pd, koffs + FB_B_OFFS, 2.0 * (cmpl(n_b, -k_b)) + 2.0 * (cmpl(n_c, -k_c)) * osc->b);
                cmpl_vector_set(pd, koffs + FB_C_OFFS, 2.0 * (cmpl(n_a, -k_a)) * osc->a * osc->c + 2.0 * (cmpl(n_c, -k_c)) * osc->c);
            }
        }
    }

    if (pd) {
        /* Derivative wrt N_inf */
        cmpl_vector_set(pd, FB_NINF_OFFS, 1.0);
        cmpl_vector_set(pd, FB_EG_OFFS, dndeg);
    }

    return cmpl(nsum, -ksum);
}

int
fb_fp_number(const disp_t *disp)
{
    return FB_NB_GLOBAL_PARAMS + disp->disp.fb.n * FB_NB_PARAMS;
}

void
fb_encode_param(str_t param, const fit_param_t *fp)
{
    if (fp->param_nb == 0) {
        str_copy_c(param, "Ninf");
    } else if (fp->param_nb == 1) {
        str_copy_c(param, "Eg");
    } else {
        int onb = (fp->param_nb - FB_NB_GLOBAL_PARAMS) / FB_NB_PARAMS;
        int pnb = (fp->param_nb - FB_NB_GLOBAL_PARAMS) % FB_NB_PARAMS;
        str_printf(param, "%s:%i", fb_param_names[pnb], onb);
    }
}

double *fb_map_param(disp_t *_d, int index)
{
    struct disp_fb *d = &_d->disp.fb;
    if (index >= d->n * FB_NB_PARAMS + FB_NB_GLOBAL_PARAMS) return nullptr;
    if (index == 0) return &d->n_inf;
    if (index == 1) return &d->eg;
    int no = (index - FB_NB_GLOBAL_PARAMS) / FB_NB_PARAMS;
    int np = (index - FB_NB_GLOBAL_PARAMS) % FB_NB_PARAMS;
    struct fb_osc *osc = d->osc + no;
    switch (np) {
        case 0: return &osc->a;
        case 1: return &osc->b;
        default: ;
    }
    return &osc->c;
}

int
fb_apply_param(struct disp_struct *disp, const fit_param_t *fp,
               double val)
{
    double *pval = fb_map_param(disp, fp->param_nb);
    if(!pval) return 1;
    *pval = val;
    return 0;
}

double
fb_get_param_value(const disp_t *d, const fit_param_t *fp)
{
    const double *pval = fb_map_param((disp_t *) d, fp->param_nb);
    assert(pval != nullptr);
    return *pval;
}

static disp_t *
disp_new_fb_generic(enum disp_type dtype, const char *name, int coeff_form, int nb_osc, double n_inf, double eg, struct fb_osc *osc)
{
    disp_t *d = disp_new_with_name(dtype, name);
    d->disp.fb.n = nb_osc;
    d->disp.fb.form = coeff_form;
    d->disp.fb.n_inf = n_inf;
    d->disp.fb.eg = eg;
    d->disp.fb.osc = (struct fb_osc *) emalloc(nb_osc * sizeof(struct fb_osc));
    memcpy(d->disp.fb.osc, osc, nb_osc * sizeof(struct fb_osc));
    return d;
}

disp_t *
disp_new_fb(const char *name, int coeff_form, int nb_osc, double n_inf, double eg, struct fb_osc *osc)
{
    return disp_new_fb_generic(DISP_FB, name, coeff_form, nb_osc, n_inf, eg, osc);
}

disp_t *
disp_new_tauc_lorentz(const char *name, int coeff_form, int nb_osc, double n_inf, double eg, struct fb_osc *osc)
{
    return disp_new_fb_generic(DISP_TAUC_LORENTZ, name, coeff_form, nb_osc, n_inf, eg, osc);
}

static void fb_change_form(struct disp_fb *fb, int new_coeff_form)
{
    if (new_coeff_form == FOROUHI_BLOOMER_STANDARD) {
        int i;
        for (i = 0; i < fb->n; i++) {
            struct fb_osc *osc = &fb->osc[i];
            const double Ap = osc->a;
            osc->a = Ap * SQR(osc->c);
            const double Bp = osc->b;
            osc->b = 2 * Bp;
            osc->c = SQR(osc->c) + SQR(Bp);
        }
    } else {
        int i;
        for (i = 0; i < fb->n; i++) {
            struct fb_osc *osc = &fb->osc[i];
            const double Qq = osc->c - SQR(osc->b) / 4;
            osc->a = osc->a / Qq;
            osc->b = osc->b / 2;
            osc->c = sqrt(Qq);
        }
    }
}

void disp_fb_change_form(disp_t *d, int new_coeff_form)
{
    struct disp_fb *fb = &d->disp.fb;
    fb->form = new_coeff_form;
    if (d->type == DISP_FB) {
        fb_change_form(fb, new_coeff_form);
    } else {
        tauc_lorentz_change_form(fb, new_coeff_form);
    }
}

int
fb_write(writer_t *w, const disp_t *_d)
{
    const struct disp_fb *d = &_d->disp.fb;
    writer_printf(w, "%d %d %g %g", d->n, d->form, d->n_inf, d->eg);
    writer_newline(w);
    struct fb_osc *fb = d->osc;
    int i;
    for (i = 0; i < d->n; i++, fb++) {
        writer_printf(w, "%g %g %g", fb->a, fb->b, fb->c);
    }
    writer_newline_exit(w);
    return 0;
}

int
fb_read(lexer_t *l, disp_t *d_gen)
{
    struct disp_fb *d = &d_gen->disp.fb;
    d->n = 0;
    d->osc = nullptr;
    int n_osc, form;
    if (lexer_integer(l, &n_osc)) return 1;
    if (lexer_integer(l, &form)) return 1;
    if (lexer_number(l, &d->n_inf)) return 1;
    if (lexer_number(l, &d->eg)) return 1;
    d->n = n_osc;
    d->form = form;
    d->osc = (struct fb_osc *) emalloc(n_osc * sizeof(struct fb_osc));
    struct fb_osc *fb = d->osc;
    int i;
    for (i = 0; i < n_osc; i++, fb++) {
        if (lexer_number(l, &fb->a)) return 1;
        if (lexer_number(l, &fb->b)) return 1;
        if (lexer_number(l, &fb->c)) return 1;
    }
    return 0;
}
