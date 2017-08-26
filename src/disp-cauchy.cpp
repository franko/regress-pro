#include <assert.h>
#include <string.h>
#include "dispers.h"
#include "cmpl.h"

static cmpl cauchy_n_value(const disp_t *disp, double lam);
static cmpl cauchy_n_value_deriv(const disp_t *disp, double lam,
                                 cmpl_vector *der);
static int  cauchy_fp_number(const disp_t *disp);
static double * cauchy_map_param(disp_t *d, int index);
static int  cauchy_apply_param(struct disp_struct *d,
                               const fit_param_t *fp, double val);
static void cauchy_encode_param(str_t param, const fit_param_t *fp);

static double cauchy_get_param_value(const struct disp_struct *d,
                                     const fit_param_t *fp);
static int cauchy_write(writer_t *w, const disp_t *_d);
static int cauchy_read(lexer_t *l, disp_t *d_gen);

#define CAUCHY_NB_N_PARAMS 3
#define CAUCHY_NB_PARAMS 6

using namespace std::complex_literals;

struct disp_class cauchy_disp_class = {
    DISP_CAUCHY, "Cauchy", "cauchy",
    disp_base_free,
    disp_base_copy,
    cauchy_n_value,
    cauchy_fp_number,
    cauchy_n_value_deriv,
    cauchy_apply_param,
    cauchy_map_param,
    cauchy_get_param_value,
    nullptr, nullptr,
    cauchy_encode_param,
    cauchy_read,
    cauchy_write,
};

static cmpl cauchy(const struct disp_cauchy *c, const double lam) {
    const double lamsq = lam * lam;
    const double n = c->n[0] + c->n[1] / lamsq + c->n[2] / (lamsq*lamsq);
    const double k = c->k[0] + c->k[1] / lamsq + c->k[2] / (lamsq*lamsq);
    return cmpl(n, -k);
}

cmpl
cauchy_n_value(const disp_t *disp, double lam)
{
    const struct disp_cauchy *c = & disp->disp.cauchy;
    return cauchy(c, lam);
}

cmpl
cauchy_n_value_deriv(const disp_t *d, double lam, cmpl_vector *pd)
{
    const struct disp_cauchy *c = & d->disp.cauchy;
    const double lamsq = lam * lam;
    const cmpl n = cauchy(c, lam);
    pd->at(0) = 1.0;
    pd->at(1) = 1.0 / lamsq;
    pd->at(2) = 1.0 / (lamsq*lamsq);
    pd->at(3) = - 1i;
    pd->at(4) = - 1i / lamsq;
    pd->at(5) = - 1i / (lamsq*lamsq);
    return n;
}

int
cauchy_fp_number(const disp_t *disp)
{
    return CAUCHY_NB_PARAMS;
}

void
cauchy_encode_param(str_t param, const fit_param_t *fp)
{
    int nb = fp->param_nb;
    str_printf(param, "%s%i", (nb < 3 ? "N" : "K"), nb % 3);
}

double *
cauchy_map_param(disp_t *_d, int index)
{
    struct disp_cauchy *d = &_d->disp.cauchy;
    if (index < 3) {
        return d->n + index;
    } else if (index < 6) {
        return d->k + (index - 3);
    }
    return nullptr;
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

int
cauchy_write(writer_t *w, const disp_t *_d)
{
    const struct disp_cauchy *d = & _d->disp.cauchy;
    writer_printf(w, "%g %g %g", d->n[0], d->n[1], d->n[2]);
    writer_newline(w);
    writer_printf(w, "%g %g %g", d->k[0], d->k[1], d->k[2]);
    writer_newline_exit(w);
    return 0;
}

int
cauchy_read(lexer_t *l, disp_t *d_gen)
{
    struct disp_cauchy *d = &d_gen->disp.cauchy;
    if (lexer_number(l, &d->n[0])) return 1;
    if (lexer_number(l, &d->n[1])) return 1;
    if (lexer_number(l, &d->n[2])) return 1;
    if (lexer_number(l, &d->k[0])) return 1;
    if (lexer_number(l, &d->k[1])) return 1;
    if (lexer_number(l, &d->k[2])) return 1;
    return 0;
}
