
/* disp-ho.c
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

static void     ho_free(disp_t *d);
static disp_t * ho_copy(const disp_t *d);

static cmpl ho_n_value(const disp_t *disp, double lam);
static cmpl ho_n_value_deriv(const disp_t *disp, double lam,
                             cmpl_vector *der);
static int  ho_fp_number(const disp_t *disp);
static int  ho_decode_param_string(const char *p);
static int  ho_apply_param(struct disp_struct *d,
                           const fit_param_t *fp, double val);
static void ho_encode_param(str_t param, const fit_param_t *fp);

static double ho_get_param_value(const struct disp_struct *d,
                                 const fit_param_t *fp);

static void ho_get_param_bounds(fit_param_t *fp, double *lower, double *upper);

struct disp_class ho_disp_class = {
    .disp_class_id       = DISP_HO,
    .model_id            = MODEL_HO,

    .short_id            = "HO",
    .full_id             = "HODispersion",

    .free                = ho_free,
    .copy                = ho_copy,

    .n_value             = ho_n_value,
    .fp_number           = ho_fp_number,
    .n_value_deriv       = ho_n_value_deriv,
    .apply_param         = ho_apply_param,
    .get_param_value     = ho_get_param_value,

    .decode_param_string = ho_decode_param_string,
    .encode_param        = ho_encode_param,
    .get_param_bounds    = ho_get_param_bounds,
};

static const char *ho_param_names[] = {"Nosc", "En", "Eg", "Nu", "Phi"};

#define HO_MULT_FACT 1.3732
#define HO_NB_PARAMS 5
#define HO_NOSC_OFFS 0
#define HO_EN_OFFS   1
#define HO_EG_OFFS   2
#define HO_NU_OFFS   3
#define HO_PHI_OFFS  4

#define HO_PARAM_NB(hn,pn) (HO_NB_PARAMS * (hn) + pn)

void
ho_free(disp_t *d)
{
    struct disp_ho *ho = & d->disp.ho;
    assert(d->type == DISP_HO);
    str_free(d->name);
    free(ho->params);
    free(d);
}

disp_t *
ho_copy(const disp_t *src)
{
    struct disp_struct *res;
    struct disp_ho *ho;
    struct ho_params *params;

    res = disp_base_copy(src);

    ho = & res->disp.ho;
    params = ho->params;
    ho->params = emalloc(sizeof(struct ho_params) * ho->nb_hos);
    memcpy(ho->params, params, sizeof(struct ho_params) * ho->nb_hos);

    return res;
}

cmpl
ho_n_value(const disp_t *disp, double lam)
{
    return ho_n_value_deriv(disp, lam, NULL);
}

cmpl
ho_n_value_deriv(const disp_t *d, double lambda, cmpl_vector *pd)
{
    const struct disp_ho *m = & d->disp.ho;
    int k, nb = m->nb_hos;
    cmpl hsum, hnusum, den;
    cmpl epsfact, n;
    double e;
    int chop_k;

    e = 1239.8 / lambda;

    hsum = 0.0, hnusum = 0.0;
    for(k = 0; k < nb; k++) {
        cmpl hh;
        const struct ho_params *p = m->params + k;

        hh = HO_MULT_FACT * p->nosc * cexp(- I * p->phi) / \
             (SQR(p->en) - SQR(e) + I * p->eg * e);

        if(pd) {
            cmpl_vector_set(pd, HO_NB_PARAMS * k + HO_NOSC_OFFS, hh / p->nosc);
            cmpl_vector_set(pd, HO_NB_PARAMS * k + HO_EN_OFFS, hh);
            cmpl_vector_set(pd, HO_NB_PARAMS * k + HO_EG_OFFS, hh);
            cmpl_vector_set(pd, HO_NB_PARAMS * k + HO_NU_OFFS, hh);
            cmpl_vector_set(pd, HO_NB_PARAMS * k + HO_PHI_OFFS, - I * hh);
        }

        hsum += hh;
        hnusum += p->nu * hh;
    }

    n = csqrt(1 + hsum/(1 - hnusum));
    chop_k = (cimag(n) > 0.0);

    den = 1 - hnusum;
    epsfact = 1 / (2.0 * csqrt(1 + hsum / den));

    if(chop_k) {
        n = creal(n) + I * 0.0;
    }

    if(pd == NULL) {
        return n;
    }

    for(k = 0; k < nb; k++) {
        const struct ho_params *p = m->params + k;
        int idx, koffs = k * HO_NB_PARAMS;
        cmpl dndh, y, hhden;

        idx = koffs + HO_NU_OFFS;
        y = hsum / SQR(den) * cmpl_vector_get(pd, idx);
        y *= epsfact;
        cmpl_vector_set(pd, idx, y);

        dndh = p->nu * hsum / SQR(den) + 1 / den;

        idx = koffs + HO_NOSC_OFFS;
        y = dndh * cmpl_vector_get(pd, idx);
        y *= epsfact;
        cmpl_vector_set(pd, idx, y);

        idx = koffs + HO_PHI_OFFS;
        y = dndh * cmpl_vector_get(pd, idx);
        y *= epsfact;
        cmpl_vector_set(pd, idx, y);

        hhden = SQR(p->en) - SQR(e) + I * p->eg * e;

        idx = koffs + HO_EN_OFFS;
        y = dndh * (- 2.0 * p->en / hhden) * cmpl_vector_get(pd, idx);
        y *= epsfact;
        cmpl_vector_set(pd, idx, y);

        idx = koffs + HO_EG_OFFS;
        y = dndh * (- I * e / hhden) * cmpl_vector_get(pd, idx);
        y *= epsfact;
        cmpl_vector_set(pd, idx, y);
    }

    if(chop_k) {
        for(k = 0; k < nb * HO_NB_PARAMS; k++) {
            cmpl y = cmpl_vector_get(pd, k);
            cmpl_vector_set(pd, k, creal(y) + I * 0.0);
        }
    }

    return n;
}

int
ho_fp_number(const disp_t *disp)
{
    return disp->disp.ho.nb_hos * HO_NB_PARAMS;
}

int
ho_decode_param_string(const char *param)
{
    const char *snext;
    char *tail;
    int j, slen, nn;

    snext = strchr(param, ':');
    if(snext == NULL) {
        return -1;
    }
    slen = snext - param;

    for(j = 0; j < HO_NB_PARAMS; j++) {
        const char *pname = ho_param_names[j];

        if(strncmp(param, pname, slen) == 0) {
            break;
        }
    }

    if(j >= HO_NB_PARAMS) {
        return -1;
    }

    param = snext + 1;

    nn = strtol(param, & tail, 10);
    if(*tail != 0 || tail == param || nn < 0) {
        return -1;
    }

    return HO_PARAM_NB(nn, j);
}

static double *
fit_param_address(struct disp_ho *d, const fit_param_t *fp)
{
    struct ho_params *ho;
    int nho, npp;

    assert(fp->param_nb < d->nb_hos * HO_NB_PARAMS);

    nho = fp->param_nb / HO_NB_PARAMS;
    npp = fp->param_nb % HO_NB_PARAMS;

    if(nho >= d->nb_hos) {
        return NULL;
    }

    ho = d->params + nho;
    switch(npp) {
    case 0:
        return &ho->nosc;
    case 1:
        return &ho->en;
    case 2:
        return &ho->eg;
    case 3:
        return &ho->nu;
    default:
        /* */
        ;
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
    struct disp_ho *d = & disp->disp.ho;
    double *pval = fit_param_address(d, fp);
    if(! pval) {
        return 1;
    }
    *pval = val;
    return 0;
}

double
ho_get_param_value(const disp_t *_d, const fit_param_t *fp)
{
    const struct disp_ho *d = & _d->disp.ho;
    const double *pval = fit_param_address((struct disp_ho *) d, fp);
    assert(pval != NULL);
    return *pval;
}

disp_t *
disp_new_ho(const char *name, int nb_hos, struct ho_params *params)
{
    disp_t *d = disp_new_with_name(DISP_HO, name);

    d->disp.ho.nb_hos = nb_hos;
    d->disp.ho.params = emalloc(nb_hos * sizeof(struct ho_params));
    memcpy(d->disp.ho.params, params, nb_hos * sizeof(struct ho_params));

    return d;
}

void
ho_get_param_bounds(fit_param_t *fp, double *lower, double *upper)
{
    int npp;
    npp = fp->param_nb % HO_NB_PARAMS;
    switch(npp) {
    case 0:
        *lower = 0.0;
        *upper = 2000.0;
        break;
    case 1:
        *lower = 0.0;
        *upper = 35.0;
        break;
    case 2:
        *lower = 0.0;
        *upper = 35.0;
        break;
    case 3:
        *lower = 0.0;
        *upper = 1.0;
        break;
    default:
        *lower = - M_PI;
        *upper = M_PI;
        break;
    }
}
