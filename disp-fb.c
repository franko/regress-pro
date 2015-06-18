
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

static void     fb_free(disp_t *d);
static disp_t * fb_copy(const disp_t *d);

static cmpl fb_n_value(const disp_t *disp, double lam);
static cmpl fb_n_value_deriv(const disp_t *disp, double lam,
                             cmpl_vector *der);
static int  fb_fp_number(const disp_t *disp);
// static int  fb_decode_param_string(const char *p);
static int  fb_apply_param(struct disp_struct *d,
                           const fit_param_t *fp, double val);
static void fb_encode_param(str_t param, const fit_param_t *fp);

static double fb_get_param_value(const struct disp_struct *d,
                                 const fit_param_t *fp);

static int fb_write(writer_t *w, const disp_t *_d);
static int fb_read(lexer_t *l, disp_t *d);

struct disp_class fb_disp_class = {
    .disp_class_id       = DISP_FB,
    .model_id            = MODEL_FB,

    .short_id            = "FB",
    .full_id             = "FBDispersion",

    .free                = fb_free,
    .copy                = fb_copy,

    .n_value             = fb_n_value,
    .fp_number           = fb_fp_number,
    .n_value_deriv       = fb_n_value_deriv,
    .apply_param         = fb_apply_param,
    .get_param_value     = fb_get_param_value,

    .decode_param_string = NULL,
    .encode_param        = fb_encode_param,
    .write               = fb_write,
    .read                = fb_read,
};

static const char *fb_param_names[] = {"Eg", "A", "B", "C"};

#define FB_EV_NM 1240.0

#define FB_NB_GLOBAL_PARAMS 1
#define FB_NINF_OFFS 0
#define FB_NB_PARAMS 4
#define FB_EG_OFFS 0
#define FB_A_OFFS  1
#define FB_B_OFFS  2
#define FB_C_OFFS  3

#define FB_PARAM_NB(hn,pn) (FB_NB_PARAMS * (hn) + pn)

void
fb_free(disp_t *d)
{
    struct disp_fb *fb = & d->disp.fb;
    str_free(d->name);
    free(fb->osc);
    free(d);
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
    fb->osc = emalloc(sizeof(struct fb_osc) * fb->n);
    memcpy(fb->osc, osc, sizeof(struct fb_osc) * fb->n);

    return res;
}

void
disp_add_osc(struct disp_struct *d)
{
    struct disp_fb *fb = &d->disp.fb;
    int n = fb->n;
    struct fb_osc *osc = emalloc(sizeof(struct fb_osc) * (n + 1));
    memcpy(osc, fb->osc, sizeof(struct fb_osc) * n);
    osc[n].eg = 15.0;
    osc[n].a = 1.0;
    osc[n].b = 0.0;
    osc[n].c = 9.0;

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
    return fb_n_value_deriv(disp, lam, NULL);
}
cmpl
fb_n_value_deriv(const disp_t *d, double lambda, cmpl_vector *pd)
{
    const struct disp_fb *fb = &d->disp.fb;
    int k, nb = fb->n;
    double nsum = fb->n_inf, ksum = 0;
    double E = FB_EV_NM / lambda;

    for(k = 0; k < nb; k++) {
        const struct fb_osc *osc = fb->osc + k;
        const double A = osc->a, B = osc->b, C = osc->c;
        const double Eg = osc->eg;
        const double den = E * (E - B) + C;
        const double Q = 0.5 * sqrt(4*C - SQR(B));
        const double B0 = (A / Q) * (-SQR(B)/2 + Eg * (B - Eg) + C);
        const double C0 = (A / Q) * ((SQR(Eg) + C)*B/2 - 2 * Eg * C);

        const double kterm = A * SQR(E - Eg) / den;
        const double nterm = (B0*E + C0) / den;

        ksum += kterm;
        nsum += nterm;

        if (pd) {
            const int koffs = FB_NB_GLOBAL_PARAMS + k * FB_NB_PARAMS;

            /* Derivatives */
            const double k_eg = -2 * A * (E - Eg) / den;
            const double n_eg = (A / Q) * (E * B - 2*E*Eg + Eg * B - 2*C) / den;
            cmpl_vector_set(pd, koffs + FB_EG_OFFS, n_eg - I * k_eg);

            const double k_a = kterm / A;
            const double n_a = nterm / A;
            cmpl_vector_set(pd, koffs + FB_A_OFFS, n_a - I * k_a);

            const double dB0dB = A * (B*SQR(B) + 8*C*Eg - 2*B*(3*C+SQR(Eg))) / (8 * Q*SQR(Q));
            const double dC0dB = A * C * (C + Eg * (Eg - B)) / (2 * Q*SQR(Q));

            const double k_b = E * kterm / den;
            const double n_b = (dB0dB * E + dC0dB) / den + E * nterm / den;
            cmpl_vector_set(pd, koffs + FB_B_OFFS, n_b - I * k_b);

            const double dB0dC = A * (C + Eg * (Eg - B)) / (2 * Q*SQR(Q));
            const double dC0dC = A * ((B - 4*Eg) * (2*C - SQR(B)) - 2*B*SQR(Eg)) / (8 * Q*SQR(Q));
            const double k_c = - kterm / den;
            const double n_c = (dB0dC * E + dC0dC) / den - nterm / den;
            cmpl_vector_set(pd, koffs + FB_C_OFFS, n_c - I * k_c);
        }
    }

    if (pd) {
        /* Derivative wrt N_inf */
        cmpl_vector_set(pd, FB_NINF_OFFS, 1.0);
    }

    return nsum - I * ksum;
}

int
fb_fp_number(const disp_t *disp)
{
    return FB_NB_GLOBAL_PARAMS + disp->disp.fb.n * FB_NB_PARAMS;
}

static double *
fit_param_address(struct disp_fb *d, const fit_param_t *fp)
{
    assert(fp->param_nb < d->n * FB_NB_PARAMS + FB_NB_GLOBAL_PARAMS);

    if (fp->param_nb == FB_NINF_OFFS) return &d->n_inf;

    int no, np;
    no = (fp->param_nb - FB_NB_GLOBAL_PARAMS) / FB_NB_PARAMS;
    np = (fp->param_nb - FB_NB_GLOBAL_PARAMS) % FB_NB_PARAMS;

    if(no >= d->n) return NULL;

    struct fb_osc *fb = d->osc + no;
    switch(np) {
    case 0: return &fb->eg;
    case 1: return &fb->a;
    case 2: return &fb->b;
    default: ;
    }
    return &fb->c;
}

void
fb_encode_param(str_t param, const fit_param_t *fp)
{
    if (fp->param_nb == 0) {
        str_copy_c(param, "Ninf");
    } else {
        int onb = (fp->param_nb - FB_NB_GLOBAL_PARAMS) / FB_NB_PARAMS;
        int pnb = (fp->param_nb - FB_NB_GLOBAL_PARAMS) % FB_NB_PARAMS;
        str_printf(param, "%s:%i", fb_param_names[pnb], onb);
    }
}

int
fb_apply_param(struct disp_struct *disp, const fit_param_t *fp,
               double val)
{
    struct disp_fb *d = &disp->disp.fb;
    double *pval = fit_param_address(d, fp);
    if(!pval) return 1;
    *pval = val;
    return 0;
}

double
fb_get_param_value(const disp_t *_d, const fit_param_t *fp)
{
    const struct disp_fb *d = &_d->disp.fb;
    const double *pval = fit_param_address((struct disp_fb *) d, fp);
    assert(pval != NULL);
    return *pval;
}

disp_t *
disp_new_fb(const char *name, int nb_osc, double n_inf, struct fb_osc *osc)
{
    disp_t *d = disp_new_with_name(DISP_FB, name);
    d->disp.fb.n = nb_osc;
    d->disp.fb.n_inf = n_inf;
    d->disp.fb.osc = emalloc(nb_osc * sizeof(struct fb_osc));
    memcpy(d->disp.fb.osc, osc, nb_osc * sizeof(struct fb_osc));
    return d;
}

int
fb_write(writer_t *w, const disp_t *_d)
{
    const struct disp_fb *d = &_d->disp.fb;
    writer_printf(w, "fb \"%s\" %d", CSTR(_d->name), d->n);
    writer_printf(w, " %g", d->n_inf);
    writer_newline_enter(w);
    struct fb_osc *fb = d->osc;
    int i;
    for (i = 0; i < d->n; i++, fb++) {
        if (i > 0) {
            writer_newline(w);
        }
        writer_printf(w, "%g %g %g %g", fb->eg, fb->a, fb->b, fb->c);
    }
    writer_newline_exit(w);
    return 0;
}

int
fb_read(lexer_t *l, disp_t *d_gen)
{
    struct disp_fb *d = &d_gen->disp.fb;
    int n_osc;
    if (lexer_integer(l, &n_osc)) return 1;
    if (lexer_number(l, &d->n_inf)) return 1;
    d->osc = NULL;
    d->n = n_osc;
    d->osc = emalloc(n_osc * sizeof(struct fb_osc));
    struct fb_osc *fb = d->osc;
    int i;
    for (i = 0; i < n_osc; i++, fb++) {
        if (lexer_number(l, &fb->eg)) return 1;
        if (lexer_number(l, &fb->a)) return 1;
        if (lexer_number(l, &fb->b)) return 1;
        if (lexer_number(l, &fb->c)) return 1;
    }
    return 0;
}
