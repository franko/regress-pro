
#include <assert.h>
#include <string.h>
#include "dispers.h"
#include "cmpl.h"

static void     bruggeman_free(struct disp_struct *d);
static disp_t * bruggeman_copy(const disp_t *d);

static cmpl bruggeman_n_value(const disp_t *disp, double lam);
static cmpl bruggeman_n_value_deriv(const disp_t *disp, double lam,
                                    cmpl_vector *der);
static int  bruggeman_fp_number(const disp_t *disp);
static int  bruggeman_apply_param(struct disp_struct *d,
                                  const fit_param_t *fp, double val);
static void bruggeman_encode_param(str_t param, const fit_param_t *fp);

static double bruggeman_get_param_value(const struct disp_struct *d,
                                        const fit_param_t *fp);

struct disp_class bruggeman_disp_class = {
    .disp_class_id       = DISP_BRUGGEMAN,
    .full_name           = "Bruggeman EMA",
    .short_name          = "bruggeman",

    .free                = bruggeman_free,
    .copy                = bruggeman_copy,

    .n_value             = bruggeman_n_value,
    .fp_number           = bruggeman_fp_number,
    .n_value_deriv       = bruggeman_n_value_deriv,
    .apply_param         = bruggeman_apply_param,
    .get_param_value     = bruggeman_get_param_value,

    .encode_param        = bruggeman_encode_param,
};

void
bruggeman_free(struct disp_struct *d)
{
    struct disp_bruggeman *bd = & d->disp.bruggeman;
    assert(d->type == DISP_BRUGGEMAN);
    str_free(d->name);
    disp_free(bd->comp[0]);
    disp_free(bd->comp[1]);
    free(d);
}

disp_t *
bruggeman_copy(const disp_t *src)
{
    struct disp_struct *res;
    struct disp_bruggeman *bd;

    res = emalloc(sizeof(struct disp_struct));
    memcpy(res, src, sizeof(struct disp_struct));
    str_init_from_str(res->name, src->name);
    bd = & res->disp.bruggeman;
    bd->comp[0] = disp_copy(bd->comp[0]);
    bd->comp[1] = disp_copy(bd->comp[1]);
    return res;
}

cmpl
bruggeman_n_value(const disp_t *disp, double lam)
{
    return bruggeman_n_value_deriv(disp, lam, NULL);
}

cmpl
bruggeman_n_value_deriv(const disp_t *disp, double lam, cmpl_vector *v)
{
    const struct disp_bruggeman *d = & disp->disp.bruggeman;
    cmpl eps, b, e1, e2, delta, n;
    double f1, f2;

    e1 = n_value(d->comp[0], lam);
    e1 *= e1;

    e2 = n_value(d->comp[1], lam);
    e2 *= e2;

    f1 = d->frac[0];
    f2 = d->frac[1];

    b = 2 * f1 * e1 + 2 * f2 * e2 - f1 * e2 - f2 * e1;
    delta = b*b + 8 * e1 * e2;

    eps = 1/4.0 * (b + csqrt(delta));
    n = csqrt(eps);

    if(v != NULL) {
        cmpl der = 1/(2*n) * (3 * eps * (e1 - e2)) / csqrt(delta);
        cmpl_vector_set(v, 0, der);
        cmpl_vector_set(v, 1, -der);
    }

    return n;
}

int
bruggeman_fp_number(const disp_t *disp)
{
    return 2;
}

void
bruggeman_encode_param(str_t param, const fit_param_t *fp)
{
    str_printf(param, "f:%i", fp->param_nb);
}

int
bruggeman_apply_param(struct disp_struct *d, const fit_param_t *fp,
                      double val)
{
    struct disp_bruggeman *bd = & d->disp.bruggeman;

    assert(d->type == DISP_BRUGGEMAN);

    if(fp->model_id != DISP_BRUGGEMAN) {
        return 1;
    }

    if(fp->param_nb == 0) {
        bd->frac[0] = val;
        bd->frac[1] = 1.0 - val;
    } else if(fp->param_nb == 1) {
        bd->frac[1] = val;
        bd->frac[0] = 1.0 - val;
    } else {
        return 1;
    }

    return 0;
}

double
bruggeman_get_param_value(const struct disp_struct *d,
                          const fit_param_t *fp)
{
    const struct disp_bruggeman *bd = & d->disp.bruggeman;
    int np = fp->param_nb;
    assert(np >= 0 && np < 2);
    return bd->frac[np];
}
