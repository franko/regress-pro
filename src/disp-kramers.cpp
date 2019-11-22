
/* disp-kramers.cpp
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
#include "math-constants.h"
#include "math-utils.h"
#include "cmpl.h"

static void     ho_free(disp_t *d);
static disp_t * ho_copy(const disp_t *d);

static cmpl ho_n_value(const disp_t *disp, double lam);
static cmpl ho_n_value_deriv(const disp_t *disp, double lam,
                             cmpl_vector *der);
static int  ho_fp_number(const disp_t *disp);
static double * ho_map_param(disp_t *d, int index);
static int  ho_apply_param(struct disp_struct *d,
                           const fit_param_t *fp, double val);
static void ho_encode_param(str_t param, const fit_param_t *fp);

static double ho_get_param_value(const struct disp_struct *d,
                                 const fit_param_t *fp);

static int ho_write(writer_t *w, const disp_t *_d);
static int ho_read(lexer_t *l, disp_t *d);

using namespace std::complex_literals;

struct disp_class ho_disp_class = {
    DISP_HO, "Harmonic Oscillator", "ho",
    ho_free,
    ho_copy,
    ho_n_value,
    ho_fp_number,
    ho_n_value_deriv,
    ho_apply_param,
    ho_map_param,
    ho_get_param_value,
    nullptr, nullptr,
    ho_encode_param,
    ho_read,
    ho_write,
};

static const char *kramers_param_names[] = {"A", "En", "Eg", "Phi"};

#define HO_EV_NM 1239.852

#define KRAMERS_OSC_OFFS 0
#define KRAMERS_NB_OSC_PARAMS 4
#define KRAMERS_A_OFFS   0
#define KRAMERS_EN_OFFS  1
#define KRAMERS_EG_OFFS  2
#define KRAMERS_PHI_OFFS 3

#define HO_PARAM_NB(hn,pn) (HO_NB_PARAMS * (hn) + pn)

void
ho_free(disp_t *d)
{
    struct disp_kramers *ho = & d->disp.ho;
    assert(d->type == DISP_HO);
    free(ho->params);
    disp_base_free(d);
}

disp_t *
ho_copy(const disp_t *src)
{
    struct disp_struct *res;
    struct disp_kramers *ho;
    struct kramers_params *params;

    res = disp_base_copy(src);

    ho = & res->disp.ho;
    params = ho->params;
    ho->params = (struct kramers_params *) emalloc(sizeof(struct kramers_params) * ho->nb_hos);
    memcpy(ho->params, params, sizeof(struct kramers_params) * ho->nb_hos);

    return res;
}

void
disp_add_ho(struct disp_struct *d)
{
    struct disp_kramers *ho = &d->disp.ho;
    int n = ho->nb_hos;
    struct kramers_params *params = (struct kramers_params *) emalloc(sizeof(struct kramers_params) * (n + 1));
    memcpy(params, ho->params, sizeof(struct kramers_params) * n);
    params[n].nosc = 0.0;
    params[n].en = 15.7;
    params[n].eg = 0.3;
    params[n].nu = 1.0 / 3.0;
    params[n].phi = 0.0;

    free(ho->params);
    ho->params = params;
    ho->nb_hos = n + 1;
}

void
disp_delete_ho(struct disp_struct *d, int index)
{
    struct disp_kramers *ho = &d->disp.ho;
    int n = ho->nb_hos;
    int nrem = n - index - 1;
    memcpy(ho->params + index, ho->params + index + 1, sizeof(struct kramers_params) * nrem);
    ho->nb_hos = n - 1;
}

cmpl
ho_n_value(const disp_t *disp, double lam)
{
    return ho_n_value_deriv(disp, lam, nullptr);
}

cmpl
ho_n_value_deriv(const disp_t *d, double lambda, cmpl_vector *pd)
{
    const struct disp_kramers *m = & d->disp.ho;
    const int nb = m->nb_hos;
    const double e = HO_EV_NM/ lambda;
    cmpl_array8 hh(nb), invhhden(nb);

    cmpl hsum = 0.0, hnusum = 0.0;
    for (int k = 0; k < nb; k++) {
        const struct kramers_params *p = m->params + k;
        const double nosc = p->nosc, en = p->en, eg = p->eg, nu = p->nu, phi = p->phi;

        invhhden[k] = 1.0 / (pow2(en) - pow2(e) + 1i * eg * e);
        hh[k] = HO_MULT_FACT * (cos(phi) - 1i * sin(phi)) * invhhden[k];

        hsum += nosc * hh[k];
        hnusum += nu * nosc * hh[k];
    }

    const cmpl invden = 1.0 / (1.0 - hnusum);
    const cmpl eps = 1.0 + hsum * invden;
    const cmpl n_raw = std::sqrt(eps);
    const int chop_k = (std::imag(n_raw) > 0.0);

    if(pd == NULL) {
        return (chop_k ? std::real(n_raw) : n_raw);
    }

    const cmpl epsfact = 1.0 / (2 * n_raw);

    const cmpl dndnuhc = epsfact * hsum * pow2(invden);
    for (int k = 0; k < nb; k++) {
        const struct kramers_params *p = m->params + k;
        const double nosc = p->nosc, en = p->en, nu = p->nu;
        const int koffs = k * HO_NB_PARAMS;

        pd->at(koffs + HO_NU_OFFS) = dndnuhc * nosc * hh[k];

        const cmpl dndh = (nu * hsum * invden + 1.0) * invden;
        const cmpl hhdndosc = epsfact * dndh * hh[k];
        const cmpl hhdndh = nosc * hhdndosc;

        pd->at(koffs + HO_NOSC_OFFS) = hhdndosc;
        pd->at(koffs + HO_PHI_OFFS) = - 1i * hhdndh;

        const cmpl depsde = hhdndh * invhhden[k];
        pd->at(koffs + HO_EN_OFFS) = - 2 * en * depsde;
        pd->at(koffs + HO_EG_OFFS) = - 1i * e * depsde;
    }

    if (chop_k) {
        for (int k = 0; k < nb * HO_NB_PARAMS; k++) {
            const cmpl y = pd->at(k);
            pd->at(k) = std::real(y);
        }
        return std::real(n_raw);
    }
    return n_raw;
}

int
ho_fp_number(const disp_t *disp)
{
    return disp->disp.ho.nb_hos * HO_NB_PARAMS;
}

int disp_ho_oscillator_parameters_number(struct disp_struct *d)
{
    return HO_NB_PARAMS;
}

double *
ho_map_param(disp_t *_d, int index)
{
    const struct disp_kramers *d = &_d->disp.ho;
    if (index >= d->nb_hos * HO_NB_PARAMS) return nullptr;
    int no = index / HO_NB_PARAMS;
    int np = index % HO_NB_PARAMS;
    struct kramers_params *ho = d->params + no;
    switch(np) {
    case 0: return &ho->nosc;
    case 1: return &ho->en;
    case 2: return &ho->eg;
    case 3: return &ho->nu;
    default: ;
    }
    return &ho->phi;
}

void
ho_encode_param(str_t param, const fit_param_t *fp)
{
    int onb = fp->param_nb / HO_NB_PARAMS;
    int pnb = fp->param_nb % HO_NB_PARAMS;
    str_printf(param, "%s:%i", ho_param_names[pnb], onb);
}

int
ho_apply_param(struct disp_struct *disp, const fit_param_t *fp,
               double val)
{
    double *pval = ho_map_param(disp, fp->param_nb);
    if(! pval) return 1;
    *pval = val;
    return 0;
}

double
ho_get_param_value(const disp_t *d, const fit_param_t *fp)
{
    const double *pval = ho_map_param((disp_t *) d, fp->param_nb);
    assert(pval != nullptr);
    return *pval;
}

int
ho_write(writer_t *w, const disp_t *_d)
{
    const struct disp_kramers *d = &_d->disp.ho;
    writer_printf(w, "%d", d->nb_hos);
    writer_newline(w);
    struct kramers_params *ho = d->params;
    int i;
    for (i = 0; i < d->nb_hos; i++, ho++) {
        if (i > 0) {
            writer_newline(w);
        }
        writer_printf(w, "%g %g %g %g %g", ho->nosc, ho->en, ho->eg, ho->nu, ho->phi);
    }
    writer_newline_exit(w);
    return 0;
}

int
ho_read(lexer_t *l, disp_t *d_gen)
{
    struct disp_kramers *d = &d_gen->disp.ho;
    d->params = nullptr;
    int n_osc;
    if (lexer_integer(l, &n_osc)) return 1;
    d->nb_hos = n_osc;
    d->params = (kramers_params *) emalloc(n_osc * sizeof(struct kramers_params));
    struct kramers_params *ho = d->params;
    int i;
    for (i = 0; i < n_osc; i++, ho++) {
        if (lexer_number(l, &ho->nosc)) return 1;
        if (lexer_number(l, &ho->en)) return 1;
        if (lexer_number(l, &ho->eg)) return 1;
        if (lexer_number(l, &ho->nu)) return 1;
        if (lexer_number(l, &ho->phi)) return 1;
    }
    return 0;
}
