
/* disp-lorentz.c
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
#include "math-utils.h"
#include "cmpl.h"

static void     lor_free(disp_t *d);
static disp_t * lor_copy(const disp_t *d);

static cmpl lor_n_value(const disp_t *disp, double lam);
static cmpl lor_n_value_deriv(const disp_t *disp, double lam,
                             cmpl_vector *der);
static int  lor_fp_number(const disp_t *disp);
static double * lor_map_param(disp_t *d, int index);
static int  lor_apply_param(struct disp_struct *d,
                           const fit_param_t *fp, double val);
static void lor_encode_param(str_t param, const fit_param_t *fp);

static double lor_get_param_value(const struct disp_struct *d,
                                 const fit_param_t *fp);

static int lor_write(writer_t *w, const disp_t *_d);
static int lor_read(lexer_t *l, disp_t *d);

using namespace std::complex_literals;

struct disp_class lorentz_disp_class = {
    DISP_LORENTZ, "Lorentz Oscillators", "lorentz",
    lor_free,
    lor_copy,
    lor_n_value,
    lor_fp_number,
    lor_n_value_deriv,
    lor_apply_param,
    lor_map_param,
    lor_get_param_value,
    nullptr, nullptr,
    lor_encode_param,
    lor_read,
    lor_write,
};

static const char *lor_param_names[] = {"A", "En", "Br"};

#define LOR_EV_NM 1239.852

#define LOR_NB_PARAMS 3
#define LOR_A_OFFS  0
#define LOR_EN_OFFS 1
#define LOR_BR_OFFS 2

void
lor_free(disp_t *d)
{
    struct disp_lorentz *lor = & d->disp.lorentz;
    assert(d->type == DISP_LORENTZ);
    free(lor->oscillators);
    disp_base_free(d);
}

disp_t *
lor_copy(const disp_t *src)
{
    struct disp_struct *res;
    struct disp_lorentz *lor;
    struct lorentz_osc *params;

    res = disp_base_copy(src);

    lor = & res->disp.lorentz;
    params = lor->oscillators;
    lor->oscillators = (struct lorentz_osc *) emalloc(sizeof(struct lorentz_osc) * lor->oscillators_number);
    memcpy(lor->oscillators, params, sizeof(struct lorentz_osc) * lor->oscillators_number);

    return res;
}

static int lor_parameters_number(const struct disp_lorentz *lor) {
    return lor->oscillators_number * LOR_NB_PARAMS + 1;
}

void
disp_lorentz_add_oscillator(struct disp_struct *d)
{
    struct disp_lorentz *lor = &d->disp.lorentz;
    int n = lor->oscillators_number;
    struct lorentz_osc *params = (struct lorentz_osc *) emalloc(sizeof(struct lorentz_osc) * (n + 1));
    memcpy(params, lor->oscillators, sizeof(struct lorentz_osc) * n);
    params[n].a = 0.0;
    params[n].en = 6.0;
    params[n].br = 0.5;

    free(lor->oscillators);
    lor->oscillators = params;
    lor->oscillators_number = n + 1;
}

void
disp_lorentz_delete_oscillator(struct disp_struct *d, int index)
{
    struct disp_lorentz *lor = &d->disp.lorentz;
    int n = lor->oscillators_number;
    int nrem = n - index - 1;
    memcpy(lor->oscillators + index, lor->oscillators + index + 1, sizeof(struct lorentz_osc) * nrem);
    lor->oscillators_number = n - 1;
}

cmpl
lor_n_value(const disp_t *disp, double lam)
{
    return lor_n_value_deriv(disp, lam, nullptr);
}

cmpl
lor_n_value_deriv(const disp_t *d, double lambda, cmpl_vector *pd)
{
    const struct disp_lorentz *lor = & d->disp.lorentz;
    const int nb = lor->oscillators_number;
    const double e = LOR_EV_NM / lambda;
    const int osc_offs = 1; /* Index offset in partial derivative for the oscillators. */
    cmpl hhinvden[nb];
    double hhnum[nb];

    cmpl hsum = 0.0;
    for(int k = 0; k < nb; k++) {
        const struct lorentz_osc *p = &lor->oscillators[k];
        const double a = p->a, br = p->br, en = p->en;
        hhnum[k] = (lor->style == LORENTZ_STYLE_ABE ? a * br * en : a * pow2(en));
        hhinvden[k] = 1.0 / (pow2(en) - pow2(e) + 1i * br * e);
        hsum += hhnum[k] * hhinvden[k];
    }

    const cmpl epsilon = lor->e_offset + hsum;
    const cmpl epsfact = 1.0 / (2.0 * std::sqrt(epsilon));
    const cmpl n = std::sqrt(epsilon);

    if(pd == nullptr) {
        return n;
    }

    /* Derivative with e_offset. */
    cmpl_vector_set(pd, 0, epsfact);

    for(int k = 0, kpd = osc_offs; k < nb; k++, kpd += LOR_NB_PARAMS) {
        const struct lorentz_osc *p = lor->oscillators + k;
        const double a = p->a, br = p->br, en = p->en;

        const cmpl epshh = epsfact * hhnum[k] * hhinvden[k];
        cmpl_vector_set(pd, kpd + LOR_A_OFFS, epshh / a);

        const cmpl epshhiden = epshh * hhinvden[k];
        const cmpl num_en = (lor->style == LORENTZ_STYLE_ABE ? - pow2(en) - pow2(e) + 1i * br * e : 2 * e * (1i * br - e));
        cmpl_vector_set(pd, kpd + LOR_EN_OFFS, epshhiden * num_en / en);

        const cmpl num_br = (lor->style == LORENTZ_STYLE_ABE ? pow2(en) - pow2(e) : - 1i * br * e);
        cmpl_vector_set(pd, kpd + LOR_BR_OFFS, epshhiden * num_br / br);
    }

    return n;
}

int
lor_fp_number(const disp_t *disp)
{
    return lor_parameters_number(&disp->disp.lorentz);
}

int disp_lorentz_oscillator_parameters_number(struct disp_struct *d)
{
    return LOR_NB_PARAMS;
}

double *
lor_map_param(disp_t *_d, int index)
{
    struct disp_lorentz *d = &_d->disp.lorentz;
    if (index >= lor_parameters_number(d)) {
        return nullptr;
    }
    if (index == 0) {
        return &d->e_offset;
    }
    const int no = (index - 1) / LOR_NB_PARAMS;
    const int np = (index - 1) % LOR_NB_PARAMS;
    struct lorentz_osc *lor = &d->oscillators[no];
    switch(np) {
    case 0: return &lor->a;
    case 1: return &lor->en;
    default: ;
    }
    return &lor->br;
}

void
lor_encode_param(str_t param, const fit_param_t *fp)
{
    const int index = fp->param_nb;
    if (index == 0) {
        str_printf(param, "einf");
    } else {
        const int onb = (index - 1) / LOR_NB_PARAMS;
        const int pnb = (index - 1) % LOR_NB_PARAMS;
        str_printf(param, "%s:%i", lor_param_names[pnb], onb);
    }
}

int
lor_apply_param(struct disp_struct *disp, const fit_param_t *fp, double val)
{
    double *pval = lor_map_param(disp, fp->param_nb);
    if(! pval) return 1;
    *pval = val;
    return 0;
}

double
lor_get_param_value(const disp_t *d, const fit_param_t *fp)
{
    const double *pval = lor_map_param((disp_t *) d, fp->param_nb);
    assert(pval != nullptr);
    return *pval;
}

disp_t *
disp_new_lorentz(const char *name, int style, int oscillators_number, struct lorentz_osc *params)
{
    disp_t *d = disp_new_with_name(DISP_LORENTZ, name);
    struct disp_lorentz *lor = &d->disp.lorentz;
    lor->style = style;
    lor->e_offset = 1.0;
    lor->oscillators_number = oscillators_number;
    lor->oscillators = (struct lorentz_osc *) emalloc(oscillators_number * sizeof(struct lorentz_osc));
    memcpy(lor->oscillators, params, oscillators_number * sizeof(struct lorentz_osc));
    return d;
}

void
disp_lorentz_change_style(disp_t *d, int new_style)
{
    struct disp_lorentz *lor = &d->disp.lorentz;
    if (lor->style == new_style) return;
    if (new_style == LORENTZ_STYLE_ABE) {
        for (int k = 0; k < lor->oscillators_number; k++) {
            struct lorentz_osc *p = &lor->oscillators[k];
            p->a = p->a * p->en / p->br;
        }
    } else {
        for (int k = 0; k < lor->oscillators_number; k++) {
            struct lorentz_osc *p = &lor->oscillators[k];
            p->a = p->a * p->br / p->en;
        }
    }
    lor->style = new_style;
}

int
lor_write(writer_t *w, const disp_t *_d)
{
    const struct disp_lorentz *d = &_d->disp.lorentz;
    writer_printf(w, "%d %g %d", (int)d->style, d->e_offset, d->oscillators_number);
    writer_newline(w);
    struct lorentz_osc *lor = d->oscillators;
    int i;
    for (i = 0; i < d->oscillators_number; i++, lor++) {
        if (i > 0) {
            writer_newline(w);
        }
        writer_printf(w, "%g %g %g", lor->a, lor->en, lor->br);
    }
    writer_newline_exit(w);
    return 0;
}

int
lor_read(lexer_t *l, disp_t *d_gen)
{
    struct disp_lorentz *d = &d_gen->disp.lorentz;
    d->oscillators = nullptr;
    int n_osc, style;
    if (lexer_integer(l, &style)) return 1;
    if (lexer_number(l, &d->e_offset)) return 1;
    if (lexer_integer(l, &n_osc)) return 1;
    d->style = style;
    d->oscillators_number = n_osc;
    d->oscillators = (struct lorentz_osc *) emalloc(n_osc * sizeof(struct lorentz_osc));
    struct lorentz_osc *lor = d->oscillators;
    int i;
    for (i = 0; i < n_osc; i++, lor++) {
        if (lexer_number(l, &lor->a)) return 1;
        if (lexer_number(l, &lor->en)) return 1;
        if (lexer_number(l, &lor->br)) return 1;
    }
    return 0;
}
