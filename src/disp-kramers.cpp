
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

static void     kramers_free(disp_t *d);
static disp_t * kramers_copy(const disp_t *d);

static cmpl kramers_n_value(const disp_t *disp, double lam);
static cmpl kramers_n_value_deriv(const disp_t *disp, double lam,
                             cmpl_vector *der);
static int  kramers_fp_number(const disp_t *disp);
static double * kramers_map_param(disp_t *d, int index);
static int  kramers_apply_param(struct disp_struct *d,
                           const fit_param_t *fp, double val);
static void kramers_encode_param(str_t param, const fit_param_t *fp);

static double kramers_get_param_value(const struct disp_struct *d,
                                 const fit_param_t *fp);

static int kramers_write(writer_t *w, const disp_t *_d);
static int kramers_read(lexer_t *l, disp_t *d);

using namespace std::complex_literals;

struct disp_class kramers_disp_class = {
    DISP_KRAMERS, "Kramers Oscillators", "kramers",
    kramers_free,
    kramers_copy,
    kramers_n_value,
    kramers_fp_number,
    kramers_n_value_deriv,
    kramers_apply_param,
    kramers_map_param,
    kramers_get_param_value,
    nullptr, nullptr,
    kramers_encode_param,
    kramers_read,
    kramers_write,
};

static const char *kramers_param_names[] = {"A", "En", "Eg", "Phi"};

#define KRAMERS_OSC_OFFS 0
#define KRAMERS_NB_OSC_PARAMS 4
#define KRAMERS_A_OFFS   0
#define KRAMERS_EN_OFFS  1
#define KRAMERS_EG_OFFS  2
#define KRAMERS_PHI_OFFS 3

#define KRAMERS_PARAM_NB(hn,pn) (KRAMERS_NB_OSC_PARAMS * (hn) + pn)

disp_t *disp_kramers_new(const char *name) {
    kramers_params *params = (kramers_params *) malloc(sizeof(kramers_params));
    if (!params) return nullptr;
    disp_t *d = disp_new_with_name(DISP_KRAMERS, name);
    d->disp.kramers.n = 1;
    d->disp.kramers.oscillators = params;
    params->a = 100;
    params->en = 12.0;
    params->eg = 0.5;
    params->phi = 0.0;
    return d;
}

void kramers_free(disp_t *d) {
    disp_kramers *kramers = & d->disp.kramers;
    assert(d->type == DISP_KRAMERS);
    free(kramers->oscillators);
    disp_base_free(d);
}

disp_t *kramers_copy(const disp_t *src) {
    disp_struct *res = disp_base_copy(src);
    disp_kramers *kramers = &res->disp.kramers;
    kramers_params *params = kramers->oscillators;
    kramers->oscillators = (kramers_params *) emalloc(sizeof(kramers_params) * kramers->n);
    memcpy(kramers->oscillators, params, sizeof(kramers_params) * kramers->n);

    return res;
}

void disp_kramers_add_osc(struct disp_struct *d) {
    disp_kramers *kramers = &d->disp.kramers;
    int n = kramers->n;
    kramers_params *params = (kramers_params *) emalloc(sizeof(kramers_params) * (n + 1));
    memcpy(params, kramers->oscillators, sizeof(kramers_params) * n);
    params[n].a = 0.0;
    params[n].en = 12.0;
    params[n].eg = 0.5;
    params[n].phi = 0.0;

    free(kramers->oscillators);
    kramers->oscillators = params;
    kramers->n = n + 1;
}

void disp_kramers_delete_osc(struct disp_struct *d, int index) {
    disp_kramers *kramers = &d->disp.kramers;
    int n = kramers->n;
    int nrem = n - index - 1;
    memcpy(kramers->oscillators + index, kramers->oscillators + index + 1, sizeof(kramers_params) * nrem);
    kramers->n = n - 1;
}

#include "kramers-kernel.cpp"
#include "kramers-diff-kernel.cpp"

int kramers_fp_number(const disp_t *disp) {
    return disp->disp.kramers.n * KRAMERS_NB_OSC_PARAMS;
}

int disp_kramers_oscillator_parameters_number(struct disp_struct *d) {
    return KRAMERS_NB_OSC_PARAMS;
}

double *kramers_map_param(disp_t *_d, int index) {
    const disp_kramers *d = &_d->disp.kramers;
    if (index >= d->n * KRAMERS_NB_OSC_PARAMS) return nullptr;
    int no = index / KRAMERS_NB_OSC_PARAMS;
    int np = index % KRAMERS_NB_OSC_PARAMS;
    kramers_params *kramers = d->oscillators + no;
    switch(np) {
    case 0: return &kramers->a;
    case 1: return &kramers->en;
    case 2: return &kramers->eg;
    default: ;
    }
    return &kramers->phi;
}

void kramers_encode_param(str_t param, const fit_param_t *fp) {
    int onb = fp->param_nb / KRAMERS_NB_OSC_PARAMS;
    int pnb = fp->param_nb % KRAMERS_NB_OSC_PARAMS;
    str_printf(param, "%s:%i", kramers_param_names[pnb], onb);
}

int kramers_apply_param(struct disp_struct *disp, const fit_param_t *fp, double val) {
    double *pval = kramers_map_param(disp, fp->param_nb);
    if(! pval) return 1;
    *pval = val;
    return 0;
}

double kramers_get_param_value(const disp_t *d, const fit_param_t *fp) {
    const double *pval = kramers_map_param((disp_t *) d, fp->param_nb);
    assert(pval != nullptr);
    return *pval;
}

int kramers_write(writer_t *w, const disp_t *_d) {
    const disp_kramers *d = &_d->disp.kramers;
    writer_printf(w, "%d", d->n);
    writer_newline(w);
    kramers_params *kramers = d->oscillators;
    for (int i = 0; i < d->n; i++, kramers++) {
        if (i > 0) {
            writer_newline(w);
        }
        writer_printf(w, "%g %g %g %g", kramers->a, kramers->en, kramers->eg, kramers->phi);
    }
    writer_newline_exit(w);
    return 0;
}

int kramers_read(lexer_t *l, disp_t *d_gen) {
    disp_kramers *d = &d_gen->disp.kramers;
    d->oscillators = nullptr;
    int n_osc;
    if (lexer_integer(l, &n_osc)) return 1;
    d->n = n_osc;
    d->oscillators = (kramers_params *) emalloc(n_osc * sizeof(kramers_params));
    kramers_params *kramers = d->oscillators;
    for (int i = 0; i < n_osc; i++, kramers++) {
        if (lexer_number(l, &kramers->a)) return 1;
        if (lexer_number(l, &kramers->en)) return 1;
        if (lexer_number(l, &kramers->eg)) return 1;
        if (lexer_number(l, &kramers->phi)) return 1;
    }
    return 0;
}
