
#include <assert.h>
#include <string.h>

#ifdef DEBUG_BRUGGEMAN
#include <gsl/gsl_deriv.h>
#endif

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv2.h>

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
static double * bruggeman_map_param(disp_t *d, int index);
static int bruggeman_write(writer_t *w, const disp_t *_d);
static int bruggeman_read(lexer_t *l, disp_t *d);

struct epsilon_fraction {
    double f;
    cmpl eps;
};

struct epsilon_sequence {
    int length;
    struct epsilon_fraction *terms;
};

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
    .map_param           = bruggeman_map_param,
    .get_param_value     = bruggeman_get_param_value,

    .encode_param        = bruggeman_encode_param,
    .write               = bruggeman_write,
    .read                = bruggeman_read,
};

void
bruggeman_free(struct disp_struct *_d)
{
    struct disp_bruggeman *d = &_d->disp.bruggeman;
    assert(_d->type == DISP_BRUGGEMAN);
    if (d->disp_base) {
        disp_free(d->disp_base);
    }
    for (int i = 0; i < d->components_number; i++) {
        disp_free(d->components[i].disp);
    }
    free(d->components);
    disp_base_free(_d);
}

disp_t *
bruggeman_copy(const disp_t *src)
{
    struct disp_struct *new_disp = disp_base_copy(src);
    const struct disp_bruggeman *src_bema = &src->disp.bruggeman;
    struct disp_bruggeman *new_bema = &new_disp->disp.bruggeman;
    new_bema->disp_base = disp_copy(src_bema->disp_base);
    new_bema->components_number = src_bema->components_number;
    new_bema->components = emalloc(src_bema->components_number * sizeof(struct bema_component));
    for (int i = 0; i < new_bema->components_number; i++) {
        new_bema->components[i].fraction = src_bema->components[i].fraction;
        new_bema->components[i].disp = disp_copy(src_bema->components[i].disp);
    }
    return new_disp;
}

/* Take the ownership of the dispersions. */
disp_t *
bruggeman_new(const char *name, disp_t *disp_base, int components_number, struct bema_component *components)
{
    disp_t *new_disp = disp_new_with_name(DISP_BRUGGEMAN, name);
    struct disp_bruggeman *bema = &new_disp->disp.bruggeman;
    bema->disp_base = disp_base;
    bema->components_number = components_number;
    bema->components = emalloc(components_number * sizeof(struct bema_component));
    memcpy(bema->components, components, components_number * sizeof(struct bema_component));
    return new_disp;
}

cmpl
bruggeman_n_value(const disp_t *disp, double lam)
{
    return bruggeman_n_value_deriv(disp, lam, NULL);
}

#ifdef DEBUG_BRUGGEMAN
static cmpl
bruggeman_debug_residual(const struct epsilon_sequence *seq, const cmpl eps, double lam) {
    cmpl sum = 0;
    for (int i = 0; i < seq->length; i++) {
        const struct epsilon_fraction *t = &seq->terms[i];
        const cmpl den = t->eps + 2 * eps;
        sum += t->f * (t->eps - eps) / den;
    }
    return sum;
}

struct deriv_eval_params {
    const disp_t *disp;
    double wavelength;
    int i;
};

double eval_real(double x, void *_p) {
    struct deriv_eval_params *p = _p;
    const struct disp_bruggeman *bema = &p->disp->disp.bruggeman;
    bema->components[p->i].fraction = x;
    const cmpl n = bruggeman_n_value_deriv(p->disp, p->wavelength, NULL);
    return creal(n);
}

double eval_imag(double x, void *_p) {
    struct deriv_eval_params *p = _p;
    const struct disp_bruggeman *bema = &p->disp->disp.bruggeman;
    bema->components[p->i].fraction = x;
    const cmpl n = bruggeman_n_value_deriv(p->disp, p->wavelength, NULL);
    return cimag(n);
}

static void
bruggeman_debug_verif_deriv(const disp_t *disp, double lam, int *VERIF_DEBUG) {
    *VERIF_DEBUG = 0;
    const struct disp_bruggeman *bema = &disp->disp.bruggeman;
    cmpl_vector *v_test = cmpl_vector_alloc(bema->components_number);
    cmpl_vector *v_num  = cmpl_vector_alloc(bema->components_number);
    bruggeman_n_value_deriv(disp, lam, v_test);

    struct deriv_eval_params deriv_params[1] = {{disp, lam, 0}};
    gsl_function F_re, F_im;
    F_re.function = &eval_real;
    F_re.params = deriv_params;
    F_im.function = &eval_imag;
    F_im.params = deriv_params;

    for (int i = 0; i < bema->components_number; i++) {
        const double f0 = bema->components[i].fraction;

        deriv_params->i = i;

        double h = 1e-4;
        double abserr;
        double deriv_re, deriv_im;
        gsl_deriv_central(&F_re, f0, h, &deriv_re, &abserr);
        bema->components[i].fraction = f0;

        gsl_deriv_central(&F_im, f0, h, &deriv_im, &abserr);
        bema->components[i].fraction = f0;

        cmpl_vector_set(v_num, i, deriv_re + I * deriv_im);
    }
    for (int i = 0; i < bema->components_number; i++) {
        const cmpl d_test = cmpl_vector_get(v_test, i);
        const cmpl d_num  = cmpl_vector_get(v_num,  i);
        printf("%3i: (%10g, %10g) (%10g, %10g)\n", i, creal(d_test), cimag(d_test), creal(d_num), cimag(d_num));
    }
    cmpl_vector_free(v_test);
    cmpl_vector_free(v_num);
    *VERIF_DEBUG = 1;
}
#endif

static int
epsilon_ode_func(double t, const double y[], double f[], void *params) {
    const struct epsilon_sequence *seq = params;
    const cmpl eps = y[0] + I * y[1];
    cmpl sum_num = 0;
    const struct epsilon_fraction *e0 = &seq->terms[0];
    const cmpl den0 = e0->eps + 2 * eps;
    cmpl sum_den = e0->f * e0->eps / (den0 * den0);
    for (int i = 1; i < seq->length; i++) {
        const struct epsilon_fraction *e = &seq->terms[i];
        const cmpl den = e->eps + 2 * eps;
        sum_num += e->f * (e->eps - eps) / den;
        sum_den += e->f * t * e->eps / (den * den);
    }
    const cmpl dedt = sum_num / (3 * sum_den);
    f[0] = creal(dedt);
    f[1] = cimag(dedt);
    return GSL_SUCCESS;
}

static void
compute_epsilon_sequence(struct epsilon_sequence *seq, const struct disp_bruggeman *d, double lam) {
    const cmpl n_base = n_value(d->disp_base, lam);
    const cmpl eps_base = n_base * n_base;
    double f_base = 1;
    for (int i = 1; i < seq->length; i++) {
        struct epsilon_fraction *t = &seq->terms[i];
        t->f = d->components[i - 1].fraction;
        const cmpl n = n_value(d->components[i - 1].disp, lam);
        t->eps = n * n;
        f_base -= t->f;
    }
    seq->terms[0].f = f_base;
    seq->terms[0].eps = eps_base;
}

/* Swap the first term of the sequence with the one having the biggest
   fraction. */
static void sequence_normalize_fraction_order(struct epsilon_sequence *seq) {
    int i_best = 0;
    double f_best = seq->terms[0].f;
    for (int i = 1; i < seq->length; i++) {
        const struct epsilon_fraction *t = &seq->terms[i];
        if (t->f > f_best) {
            i_best = i;
            f_best = t->f;
        }
    }
    if (i_best != 0) {
        struct epsilon_fraction tmp = seq->terms[0];
        seq->terms[0] = seq->terms[i_best];
        seq->terms[i_best] = tmp;
    }
}

cmpl
bruggeman_n_value_deriv(const disp_t *disp, double lam, cmpl_vector *v)
{
    const struct disp_bruggeman *d = &disp->disp.bruggeman;

    const int terms_no = d->components_number + 1;
    struct epsilon_fraction terms[terms_no];
    struct epsilon_sequence seq[1] = {{terms_no, terms}};
    compute_epsilon_sequence(seq, d, lam);

    sequence_normalize_fraction_order(seq);

    gsl_odeiv2_system sys = {epsilon_ode_func, NULL, 2, seq};
    gsl_odeiv2_driver *ode_driver = gsl_odeiv2_driver_alloc_y_new(&sys, gsl_odeiv2_step_rkf45, 1e-4, 1e-4, 0.0);
    double t = 0.0, t_final = 1.0;
    const cmpl eps0 = seq->terms[0].eps;
    double eps_data[2] = {creal(eps0), cimag(eps0)};
    int status = gsl_odeiv2_driver_apply(ode_driver, &t, t_final, eps_data);
    if (status != GSL_SUCCESS) {
      printf ("error, return value=%d\n", status);
    }
    const cmpl eps = eps_data[0] + I * eps_data[1];
    return csqrt(eps);
}

int
bruggeman_fp_number(const disp_t *disp)
{
    const struct disp_bruggeman *d = & disp->disp.bruggeman;
    return d->components_number;
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
    bd->components[fp->param_nb].fraction = val;
    return 0;
}

double
bruggeman_get_param_value(const struct disp_struct *d,
                          const fit_param_t *fp)
{
    const struct disp_bruggeman *bd = & d->disp.bruggeman;
    return bd->components[fp->param_nb].fraction;
}

double *
bruggeman_map_param(disp_t *d, int index) {
    struct disp_bruggeman *bd = & d->disp.bruggeman;
    return &bd->components[index].fraction;
}

int bruggeman_write(writer_t *w, const disp_t *d) {
    const struct disp_bruggeman *bema = &d->disp.bruggeman;
    writer_printf(w, "%d", bema->components_number);
    writer_newline_enter(w);
    disp_write(w, bema->disp_base);
    for (int i = 0; i < bema->components_number; i++) {
        writer_printf(w, "%g", bema->components[i].fraction);
        writer_newline(w);
        disp_write(w, bema->components[i].disp);
    }
    writer_indent(w, -1);
    return 0;
}

int bruggeman_read(lexer_t *l, disp_t *d) {
    struct disp_bruggeman *bema = &d->disp.bruggeman;
    bema->disp_base = NULL;
    bema->components_number = 0;
    bema->components = NULL;
    int components_number;
    if (lexer_integer(l, &components_number)) return 1;
    bema->disp_base = disp_read(l);
    if (!bema->disp_base) return 1;
    struct bema_component *components = emalloc(components_number * sizeof(struct bema_component));
    int i;
    for (i = 0; i < components_number; i++) {
        if (lexer_number(l, &components[i].fraction)) goto read_exit_clean;
        components[i].disp = disp_read(l);
        if (!components[i].disp) goto read_exit_clean;
    }
    bema->components_number = components_number;
    bema->components = components;
    return 0;
read_exit_clean:
    for (int k = 0; k < i; k++) {
        disp_free(components[k].disp);
    }
    free(components);
    return 1;
}

void
bruggeman_add_comp(disp_t *disp, int index, disp_t *new_comp) {
    struct disp_bruggeman *d = &disp->disp.bruggeman;
    const int n = d->components_number;
    struct bema_component *new_components = malloc(sizeof(struct bema_component) * (n + 1));
    memcpy(new_components, d->components, sizeof(struct bema_component) * index);
    memcpy(new_components + index + 1, d->components + index, sizeof(struct bema_component) * (n - index));
    d->components = new_components;
    d->components[index].fraction = 0.0;
    d->components[index].disp = new_comp;
    d->components_number ++;
}

void bruggeman_delete_comp(disp_t *disp, int index) {
    struct disp_bruggeman *d = &disp->disp.bruggeman;
    const int n = d->components_number;
    memmove(d->components + index, d->components + index + 1, (n - index - 1) * sizeof(struct bema_component));
    d->components_number --;
}
