
#include <assert.h>
#include <string.h>

#ifdef DEBUG_BRUGGEMAN
#include <gsl/gsl_deriv.h>
#endif

#include "dispers.h"
#include "cmpl.h"

struct epsilon_fraction {
    double f;
    cmpl eps;
};

struct epsilon_sequence {
    int length;
    struct epsilon_fraction *terms;
};

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
static cmpl bruggeman_bicomp_quad_solve(const struct epsilon_sequence *seq, double lam);
static cmpl bruggeman_epsilon_ode_newton(struct epsilon_sequence *seq, double lam);

static inline double dmin(double a, double b) {
    return (a > b ? b : a);
}

static inline double dmax(double a, double b) {
    return (a > b ? a : b);
}

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

static cmpl epsilon_sqrt(const cmpl epsilon) {
    const double THETA_TOL = M_PI / 4;
    const double er = creal(epsilon), ei = cimag(epsilon);
    const double r = sqrt(er * er + ei * ei);
    double theta = atan2(ei, er);
    if (theta > M_PI - THETA_TOL) {
        theta -= 2 * M_PI;
    }
    const double rr = sqrt(r);
    return rr * cos(theta / 2) + I * rr * sin(theta / 2);
}

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

static cmpl
epsilon_ode_eval(const struct epsilon_sequence *seq, double t, const cmpl eps) {
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
    return sum_num / (3 * sum_den);
}

struct bruggeman_rkf45 {
    const struct epsilon_sequence *seq;
    cmpl y0;
    cmpl y;
    cmpl yerr;
};

/* Runge-Kutta-Fehlberg coefficients. Zero elements left out */

static const double ah[] =
  { 1.0 / 4.0, 3.0 / 8.0, 12.0 / 13.0, 1.0, 1.0 / 2.0 };
static const double b3[] = { 3.0 / 32.0, 9.0 / 32.0 };
static const double b4[] =
  { 1932.0 / 2197.0, -7200.0 / 2197.0, 7296.0 / 2197.0 };
static const double b5[] =
  { 8341.0 / 4104.0, -32832.0 / 4104.0, 29440.0 / 4104.0, -845.0 / 4104.0 };
static const double b6[] =
  { -6080.0 / 20520.0, 41040.0 / 20520.0, -28352.0 / 20520.0,
  9295.0 / 20520.0, -5643.0 / 20520.0
};

static const double c1 = 902880.0 / 7618050.0;
static const double c3 = 3953664.0 / 7618050.0;
static const double c4 = 3855735.0 / 7618050.0;
static const double c5 = -1371249.0 / 7618050.0;
static const double c6 = 277020.0 / 7618050.0;

/* These are the differences of fifth and fourth order coefficients
   for error estimation */

static const double ec[] = { 0.0,
  1.0 / 360.0,
  0.0,
  -128.0 / 4275.0,
  -2197.0 / 75240.0,
  1.0 / 50.0,
  2.0 / 55.0
};

static void rkf45_apply (struct bruggeman_rkf45 *state, double t, double h,
             const cmpl dydt_in, cmpl *dydt_out)
{
  cmpl k1, k2, k3, k4, k5, k6;
  cmpl ytmp;

  state->y0 = state->y;
  const cmpl y = state->y;

  /* k1 step */
  k1 = dydt_in;
  ytmp = y + ah[0] * h * k1;

  /* k2 step */
  k2 = epsilon_ode_eval(state->seq, t + ah[0] * h, ytmp);
  ytmp = y + h * (b3[0] * k1 + b3[1] * k2);

  /* k3 step */
  k3 = epsilon_ode_eval(state->seq, t + ah[1] * h, ytmp);
  ytmp = y + h * (b4[0] * k1 + b4[1] * k2 + b4[2] * k3);

  /* k4 step */
  k4 = epsilon_ode_eval(state->seq, t + ah[2] * h, ytmp);
  ytmp = y + h * (b5[0] * k1 + b5[1] * k2 + b5[2] * k3 + b5[3] * k4);

  /* k5 step */
  k5 = epsilon_ode_eval(state->seq, t + ah[3] * h, ytmp);
  ytmp = y + h * (b6[0] * k1 + b6[1] * k2 + b6[2] * k3 + b6[3] * k4 + b6[4] * k5);

  /* k6 step and final sum */
  k6 = epsilon_ode_eval(state->seq, t + ah[4] * h, ytmp);
  state->y = y + h * (c1 * k1 + c3 * k3 + c4 * k4 + c5 * k5 + c6 * k6);

  /* Derivatives at output */
  *dydt_out = epsilon_ode_eval(state->seq, t + h, state->y);

  /* difference between 4th and 5th order */
  state->yerr = h * (ec[1] * k1 + ec[3] * k3 + ec[4] * k4 + ec[5] * k5 + ec[6] * k6);
}

static unsigned int
compute_epsilon_sequence(struct epsilon_sequence *seq, const struct disp_bruggeman *d, double lam) {
    const cmpl n_base = n_value(d->disp_base, lam);
    const cmpl eps_base = n_base * n_base;
    double f_base = 1;
    int eps_physical = 1;
    for (int i = 1; i < seq->length; i++) {
        struct epsilon_fraction *t = &seq->terms[i];
        t->f = d->components[i - 1].fraction;
        const cmpl n = n_value(d->components[i - 1].disp, lam);
        t->eps = n * n;
        f_base -= t->f;
        eps_physical = eps_physical && (t->f >= 0 && cimag(t->eps) - 1e-12 <= 0);
    }
    seq->terms[0].f = f_base;
    seq->terms[0].eps = eps_base;
    return eps_physical && (f_base >= 0 && cimag(eps_base) - 1e-12 <= 0);
}

static void sequence_swap_term(struct epsilon_sequence *seq, int i) {
    if (i > 0) {
        struct epsilon_fraction tmp = seq->terms[0];
        seq->terms[0] = seq->terms[i];
        seq->terms[i] = tmp;
    }
}

/* Swap the first term of the sequence with the one having the biggest
   fraction. */
static int sequence_normalize_fraction_order(struct epsilon_sequence *seq) {
    int i_best = 0;
    double f_best = seq->terms[0].f;
    for (int i = 1; i < seq->length; i++) {
        const struct epsilon_fraction *t = &seq->terms[i];
        if (t->f > f_best) {
            i_best = i;
            f_best = t->f;
        }
    }
    sequence_swap_term(seq, i_best);
    return i_best;
}

static cmpl compute_sum_fesq(const struct epsilon_sequence *seq, const cmpl eps) {
    cmpl sum_fesq = 0;
    for (int i = 0; i < seq->length; i++) {
        const struct epsilon_fraction *t = &seq->terms[i];
        const cmpl den = t->eps + 2 * eps;
        sum_fesq += 3 * t->f * t->eps / (den * den);
    }
    return sum_fesq;
}

static cmpl epsilon_ode_solve(const struct epsilon_sequence *seq, const double ytol) {
    const cmpl eps0 = seq->terms[0].eps;
    struct bruggeman_rkf45 state[1] = {{seq, eps0, eps0}};
    double t = 0.0, t_stop = 1.0;
    cmpl dydt0 = epsilon_ode_eval(state->seq, t, eps0), dydt1;
    const double h_start = 0.21;
    while (t < t_stop) {
        double t1 = dmin(t + h_start, t_stop);
        for (int k = 0; k < 20; k++) {
            rkf45_apply(state, t, t1 - t, dydt0, &dydt1);
            if (fabs(creal(state->yerr)) > ytol || fabs(cimag(state->yerr)) > ytol) {
                t1 = t + (t1 - t) / 2;
                state->y = state->y0;
            } else {
                break;
            }
        }
        t = t1;
        dydt0 = dydt1;
    }
    return state->y;
}

static cmpl epsilon_newton_refine(const struct epsilon_sequence *seq, cmpl eps, const double eps_tol) {
    int k;
    for (k = 0; k < 20; k++) {
        cmpl dsum = 0, nsum = 0;
        for (int i = 0; i < seq->length; i++) {
            const struct epsilon_fraction *t = &seq->terms[i];
            const cmpl den = t->eps + 2 * eps;
            dsum += t->f * t->eps / (den * den);
            nsum += t->f * (t->eps - eps) / den;
        }
        /* Derivative wrt epsilon is -3 * dsum at this point. */
        cmpl eps1 = eps + nsum / (3 * dsum);
        const double delta_eps = eps1 - eps;
        eps = eps1;
        if (fabs(creal(delta_eps)) < eps_tol && fabs(cimag(delta_eps)) < eps_tol) {
            break;
        }
    }
    return eps;
}

static double epsilon_abs_magnitude(const struct epsilon_sequence *seq) {
    double eps_abs = 0.0;
    for (int i = 0; i < seq->length; i++) {
        const struct epsilon_fraction *t = &seq->terms[i];
        const double er = fabs(creal(t->eps)), ei = fabs(cimag(t->eps));
        eps_abs = dmax(er, eps_abs);
        eps_abs = dmax(ei, eps_abs);;
    }
    return eps_abs;
}

cmpl bruggeman_epsilon_ode_newton(struct epsilon_sequence *seq, double lam) {
    int i_swap = sequence_normalize_fraction_order(seq);

    const double eps_abs = epsilon_abs_magnitude(seq);
    const cmpl eps_ode = epsilon_ode_solve(seq, dmin(1e-3, eps_abs * 1e-3));

    /* Re-swaps sequence terms in the correct order to compute the derivative. */
    sequence_swap_term(seq, i_swap);

    return epsilon_newton_refine(seq, eps_ode, eps_abs * 1e-8);
}

/* Assume the bruggeman has only two components (including the base). */
cmpl bruggeman_bicomp_quad_solve(const struct epsilon_sequence *seq, double lam) {
    const cmpl e1 = seq->terms[0].eps, e2 = seq->terms[1].eps;
    const double f1 = seq->terms[0].f, f2 = seq->terms[1].f;
    const cmpl b = 2 * f1 * e1 + 2 * f2 * e2 - f1 * e2 - f2 * e1;
    const cmpl root = csqrt(b*b + 8 * e1 * e2);
    const int sign = cimag(root) <= 0.0 ? 1 : -1;
    return 1/4.0 * (b + sign * root);
}

cmpl
bruggeman_n_value_deriv(const disp_t *disp, double lam, cmpl_vector *v)
{
    const struct disp_bruggeman *d = &disp->disp.bruggeman;
    const int terms_no = d->components_number + 1;
    struct epsilon_fraction terms[terms_no];
    struct epsilon_sequence seq[1] = {{terms_no, terms}};
    int is_physical = compute_epsilon_sequence(seq, d, lam);

    cmpl eps;
    if (d->components_number == 1 && is_physical) {
        eps = bruggeman_bicomp_quad_solve(seq, lam);
    } else {
        eps = bruggeman_epsilon_ode_newton(seq, lam);
    }

    const cmpl n = epsilon_sqrt(eps);
    if (v != NULL) {
        const cmpl sum_fesq = compute_sum_fesq(seq, eps);
        const cmpl deriv_factor = 1.0 / (2.0 * n);
        cmpl n_der0;
        for (int i = 0; i < seq->length; i++) {
            const struct epsilon_fraction *t = &seq->terms[i];
            const cmpl resid = (t->eps - eps) / (t->eps + 2 * eps);
            const cmpl n_der = deriv_factor * (resid / sum_fesq + eps);
            if (i == 0) {
                n_der0 = n_der;
            } else {
                cmpl_vector_set(v, i - 1, n_der - n_der0);
            }
        }
    }

    return n;
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
