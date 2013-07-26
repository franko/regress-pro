#include <assert.h>
#include <string.h>
#include "dispers.h"
#include "cmpl.h"

static cmpl cauchy_n_value(const disp_t *disp, double lam);
static cmpl cauchy_n_value_deriv(const disp_t *disp, double lam,
                                 cmpl_vector *der);
static int  cauchy_fp_number(const disp_t *disp);
static int  cauchy_decode_param_string(const char *p);
static int  cauchy_apply_param(struct disp_struct *d,
                               const fit_param_t *fp, double val);
static void cauchy_encode_param(str_t param, const fit_param_t *fp);

static double cauchy_get_param_value(const struct disp_struct *d,
                                     const fit_param_t *fp);

static void cauchy_get_param_bounds(fit_param_t *fp, double *lower, double *upper);

#define CAUCHY_NB_N_PARAMS 3
#define CAUCHY_NB_PARAMS 6

struct disp_class cauchy_disp_class = {
    .disp_class_id       = DISP_CAUCHY,
    .model_id            = MODEL_CAUCHY,

    .short_id            = "Cauchy",
    .full_id             = "Cauchy",

    .free                = disp_base_free,
    .copy                = disp_base_copy,

    .n_value             = cauchy_n_value,
    .fp_number           = cauchy_fp_number,
    .n_value_deriv       = cauchy_n_value_deriv,
    .apply_param         = cauchy_apply_param,
    .get_param_value     = cauchy_get_param_value,

    .decode_param_string = cauchy_decode_param_string,
    .encode_param        = cauchy_encode_param,
    .get_param_bounds    = cauchy_get_param_bounds,
};

cmpl
cauchy_n_value(const disp_t *disp, double lam)
{
    const struct disp_cauchy *c = & disp->disp.cauchy;
    double lamsq = lam * lam;
    cmpl n;

    n = (c->n[0] + c->n[1] / lamsq + c->n[2] / (lamsq*lamsq))		\
        - I * (c->k[0] + c->k[1] / lamsq + c->k[2] / (lamsq*lamsq));

    return n;
}

cmpl
cauchy_n_value_deriv(const disp_t *d, double lam, cmpl_vector *pd)
{
    const struct disp_cauchy *c = & d->disp.cauchy;
    double lamsq = lam * lam;
    cmpl n;

    n = (c->n[0] + c->n[1] / lamsq + c->n[2] / (lamsq*lamsq))		\
        - I * (c->k[0] + c->k[1] / lamsq + c->k[2] / (lamsq*lamsq));

    cmpl_vector_set(pd, 0, 1.0 + I * 0.0);
    cmpl_vector_set(pd, 1, 1 / lamsq + I * 0.0);
    cmpl_vector_set(pd, 2, 1 / (lamsq*lamsq) + I * 0.0);
    cmpl_vector_set(pd, 3, - I);
    cmpl_vector_set(pd, 4, - I / lamsq);
    cmpl_vector_set(pd, 5, - I / (lamsq*lamsq));

    return n;
}

int
cauchy_fp_number(const disp_t *disp)
{
    return CAUCHY_NB_PARAMS;
}

int
cauchy_decode_param_string(const char *param)
{
    char *tail;
    int kcoeff = 0, n;

    switch(param[0]) {
    case 'K':
        kcoeff = 1;
    case 'N':
        param ++;
        n = strtol(param, & tail, 10);
        if(*tail != 0 || tail == param) {
            break;
        }
        if(n > 2 || n < 0) {
            break;
        }
        return (kcoeff ? 3 : 0) + n;
    default:
        /* */
        ;
    }

    return -1;
}

void
cauchy_encode_param(str_t param, const fit_param_t *fp)
{
    int nb = fp->param_nb;
    str_printf(param, "%s%i", (nb < 3 ? "N" : "K"), nb % 3);
}

int
cauchy_apply_param(struct disp_struct *disp, const fit_param_t *fp,
                   double val)
{
    struct disp_cauchy *d = & disp->disp.cauchy;
    int nb = fp->param_nb;

    if(nb < 3) {
        d->n[nb] = val;
    } else {
        d->k[nb % 3] = val;
    }

    return 0;
}

double
cauchy_get_param_value(const struct disp_struct *_d, const fit_param_t *fp)
{
    const struct disp_cauchy *d = & _d->disp.cauchy;
    int nb = fp->param_nb;

    if(nb < 3) {
        return d->n[nb];
    }

    return d->k[nb % 3];
}

disp_t *
disp_new_cauchy(const char *name, const double n[], const double k[])
{
    disp_t *d = disp_new_with_name(DISP_CAUCHY, name);
    int j;

    for(j = 0; j < CAUCHY_NB_N_PARAMS; j++) {
        d->disp.cauchy.n[j] = n[j];
        d->disp.cauchy.k[j] = k[j];
    }

    return d;
}

void
cauchy_get_param_bounds(fit_param_t *fp, double *lower, double *upper)
{
    int nb = (fp->param_nb % 3);
    switch (nb) {
    case 0:
        *lower = 0.1;
        *upper = 8.0;
        break;
    case 1:
        *lower = -1.0e6;
        *upper = +1.0e6;
        break;
    default:
        *lower = -1.0e10;
        *upper = +1.0e10;
        break;
     }
}
