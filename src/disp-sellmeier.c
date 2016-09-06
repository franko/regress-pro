#include <assert.h>
#include <string.h>
#include "dispers.h"
#include "common.h"
#include "cmpl.h"

static cmpl sellmeier_n_value(const disp_t *disp, double lam);
static cmpl sellmeier_n_value_deriv(const disp_t *disp, double lam,
                                 cmpl_vector *der);
static int  sellmeier_fp_number(const disp_t *disp);
static double * sellmeier_map_param(disp_t *d, int index);
static int  sellmeier_apply_param(struct disp_struct *d,
                               const fit_param_t *fp, double val);
static void sellmeier_encode_param(str_t param, const fit_param_t *fp);

static double sellmeier_get_param_value(const struct disp_struct *d,
                                     const fit_param_t *fp);
static int sellmeier_write(writer_t *w, const disp_t *_d);
static int sellmeier_read(lexer_t *l, disp_t *d_gen);

#define SELLMEIER_PARAMS_NO 3

struct disp_class sellmeier_disp_class = {
    .disp_class_id       = DISP_SELLMEIER,
    .full_name           = "sellmeier",
    .short_name          = "sellmeier",

    .free                = disp_base_free,
    .copy                = disp_base_copy,

    .n_value             = sellmeier_n_value,
    .fp_number           = sellmeier_fp_number,
    .n_value_deriv       = sellmeier_n_value_deriv,
    .apply_param         = sellmeier_apply_param,
    .map_param           = sellmeier_map_param,
    .get_param_value     = sellmeier_get_param_value,

    .encode_param        = sellmeier_encode_param,
    .write               = sellmeier_write,
    .read                = sellmeier_read,
};

cmpl
sellmeier_n_value(const disp_t *disp, double lam)
{
    const struct disp_sellmeier *c = & disp->disp.sellmeier;
    const double lamsq = SQR(lam / 1000);
    double nsq = 1;
    for (int i = 0; i < SELLMEIER_PARAMS_NO; i++) {
        nsq += c->a[i] * lamsq / (lamsq - c->b[i]);
    }
    return sqrt(nsq);
}

cmpl
sellmeier_n_value_deriv(const disp_t *d, double lam, cmpl_vector *pd)
{
    const struct disp_sellmeier *c = & d->disp.sellmeier;
    const double lamsq = SQR(lam / 1000);
    double den[3];
    double nsq = 1;
    for (int i = 0; i < SELLMEIER_PARAMS_NO; i++) {
        den[i] = lamsq - c->b[i];
        nsq += c->a[i] * lamsq / den[i];
    }
    const double n = sqrt(nsq);
    const double der_coeff = 1.0 / (2 * n);
    cmpl_vector_set(pd, 0, der_coeff * lamsq / den[0]);
    cmpl_vector_set(pd, 1, der_coeff * lamsq / den[1]);
    cmpl_vector_set(pd, 2, der_coeff * lamsq / den[2]);
    cmpl_vector_set(pd, 3, der_coeff * c->a[0] * lamsq / SQR(den[0]));
    cmpl_vector_set(pd, 4, der_coeff * c->a[1] * lamsq / SQR(den[1]));
    cmpl_vector_set(pd, 5, der_coeff * c->a[2] * lamsq / SQR(den[2]));
    return n;
}

int
sellmeier_fp_number(const disp_t *disp)
{
    return 2 * SELLMEIER_PARAMS_NO;
}

void
sellmeier_encode_param(str_t param, const fit_param_t *fp)
{
    int nb = fp->param_nb;
    str_printf(param, "%s%i", (nb < SELLMEIER_PARAMS_NO ? "A" : "B"), (nb % 3) + 1);
}

double *
sellmeier_map_param(disp_t *_d, int index)
{
    struct disp_sellmeier *d = &_d->disp.sellmeier;
    if (index < SELLMEIER_PARAMS_NO) {
        return d->a + index;
    } else if (index < 2 * SELLMEIER_PARAMS_NO) {
        return d->b + (index - SELLMEIER_PARAMS_NO);
    }
    return NULL;
}

int
sellmeier_apply_param(struct disp_struct *disp, const fit_param_t *fp,
                   double val)
{
    struct disp_sellmeier *d = & disp->disp.sellmeier;
    int index = fp->param_nb;
    if(index < SELLMEIER_PARAMS_NO) {
        d->a[index] = val;
    } else {
        d->b[index - SELLMEIER_PARAMS_NO] = val;
    }
    return 0;
}

double
sellmeier_get_param_value(const struct disp_struct *_d, const fit_param_t *fp)
{
    const struct disp_sellmeier *d = & _d->disp.sellmeier;
    int index = fp->param_nb;
    return (index < SELLMEIER_PARAMS_NO ? d->a[index] : d->b[index - SELLMEIER_PARAMS_NO]);
}

disp_t *
disp_new_sellmeier(const char *name, const double a[], const double b[])
{
    disp_t *d = disp_new_with_name(DISP_SELLMEIER, name);
    for(int j = 0; j < SELLMEIER_PARAMS_NO; j++) {
        d->disp.sellmeier.a[j] = a[j];
        d->disp.sellmeier.b[j] = b[j];
    }
    return d;
}

int
sellmeier_write(writer_t *w, const disp_t *_d)
{
    const struct disp_sellmeier *d = & _d->disp.sellmeier;
    writer_printf(w, "%g %g %g", d->a[0], d->a[1], d->a[2]);
    writer_newline(w);
    writer_printf(w, "%g %g %g", d->b[0], d->b[1], d->b[2]);
    writer_newline_exit(w);
    return 0;
}

int
sellmeier_read(lexer_t *l, disp_t *d_gen)
{
    struct disp_sellmeier *d = &d_gen->disp.sellmeier;
    if (lexer_number(l, &d->a[0])) return 1;
    if (lexer_number(l, &d->a[1])) return 1;
    if (lexer_number(l, &d->a[2])) return 1;
    if (lexer_number(l, &d->b[0])) return 1;
    if (lexer_number(l, &d->b[1])) return 1;
    if (lexer_number(l, &d->b[2])) return 1;
    return 0;
}
