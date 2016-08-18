
#include <assert.h>
#include <string.h>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>

#ifdef DEBUG_BRUGGEMAN
#include <gsl/gsl_deriv.h>
#endif

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
bruggeman_debug_verif(const struct disp_bruggeman *d, const cmpl eps, double lam) {
    double f_base = 1.0;
    for (int i = 0; i < d->components_number; i++) {
        f_base -= d->components[i].fraction;
    }
    const cmpl n_base = n_value(d->disp_base, lam);
    const cmpl eps_base = n_base * n_base;
    const cmpl den_base = eps_base + 2 * eps;
    cmpl sum = f_base * (eps_base - eps) / den_base;

    for (int i = 0; i < d->components_number; i++) {
        const cmpl n_i = n_value(d->components[i].disp, lam);
        const double f_i = d->components[i].fraction;
        const cmpl eps_i = n_i * n_i;
        const cmpl den_i = eps_i + 2 * eps;
        sum += f_i * (eps_i - eps) / den_i;
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

struct epsilon_fraction {
    double f;
    cmpl eps;
};

struct epsilon_sequence {
    int length;
    struct epsilon_fraction *terms;
};

static cmpl epsilon_eqs_residual(const struct epsilon_sequence *seq, cmpl eps) {
    cmpl sum = 0;
    for (int i = 0; i < seq->length; i++) {
        const struct epsilon_fraction *t = &seq->terms[i];
        sum += t->f * (t->eps - eps) / (t->eps + 2 * eps);
    }
    return sum;
}

static double
bruggeman_epsilon_f(const gsl_vector *x, void *params) {
    const struct epsilon_sequence *seq = params;
    const cmpl eps = gsl_vector_get(x, 0) + I * gsl_vector_get(x, 1);
    const cmpl resid = epsilon_eqs_residual(seq, eps);
    return sqrt(CSQABS(resid)) + (cimag(eps) < 0 ? 0 : 10 * cimag(eps));
}

#if 0
static int
bruggeman_epsilon_df(const gsl_vector *x, void *params, gsl_matrix *J) {
    const struct epsilon_sequence *seq = params;
    const cmpl eps = gsl_vector_get(x, 0) + I * gsl_vector_get(x, 1);
    if (cimag(eps) > 0.0) return GSL_EDOM;
    cmpl sum = 0;
    for (int i = 0; i < seq->length; i++) {
        const struct epsilon_fraction *t = &seq->terms[i];
        const cmpl den = t->eps + 2 * eps;
        sum += t->f * t->eps / (den * den);
    }
    sum *= -3;

    gsl_matrix_set(J, 0, 0,  creal(sum));
    gsl_matrix_set(J, 1, 0,  cimag(sum));
    gsl_matrix_set(J, 0, 1, -cimag(sum));
    gsl_matrix_set(J, 1, 1,  creal(sum));

    return GSL_SUCCESS;
}

static int bruggeman_epsilon_fdf(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *J) {
    const struct epsilon_sequence *seq = params;
    const cmpl eps = gsl_vector_get(x, 0) + I * gsl_vector_get(x, 1);
    if (cimag(eps) > 0.0) return GSL_EDOM;
    cmpl sum = 0, sum_der = 0;
    for (int i = 0; i < seq->length; i++) {
        const struct epsilon_fraction *t = &seq->terms[i];
        const cmpl den = t->eps + 2 * eps;
        sum += t->f * (t->eps - eps) / den;
        sum_der += t->f * t->eps / (den * den);
    }
    sum_der *= -3;

    gsl_vector_set(f, 0, creal(sum));
    gsl_vector_set(f, 1, cimag(sum));

#if 0
    // DEBUG CODE
    const cmpl eps0 = eps;
    const cmpl resid0 = epsilon_eqs_residual(seq, eps0);
    double Jnum[4];
    gsl_matrix_view Jnum_view = gsl_matrix_view_array(Jnum, 2, 2);
    const double del_eps = 0.001;
    for (int j = 0; j < 2; j++) {
        const cmpl eps1 = creal(eps0) + (j == 0 ? del_eps : 0.0) + I * (cimag(eps0) + (j == 0 ? 0.0 : del_eps));
        cmpl drde = (epsilon_eqs_residual(seq, eps1) - resid0) / del_eps;
        gsl_matrix_set(&Jnum_view.matrix, 0, j, creal(drde));
        gsl_matrix_set(&Jnum_view.matrix, 1, j, cimag(drde));
    }
    printf("numeric:\n");
    printf("%g %g\n", gsl_matrix_get(&Jnum_view.matrix, 0, 0), gsl_matrix_get(&Jnum_view.matrix, 0, 1));
    printf("%g %g\n", gsl_matrix_get(&Jnum_view.matrix, 1, 0), gsl_matrix_get(&Jnum_view.matrix, 1, 1));
    printf("\n");
    // END DEBUG CODE
#endif

    gsl_matrix_set(J, 0, 0,  creal(sum_der));
    gsl_matrix_set(J, 1, 0,  cimag(sum_der));
    gsl_matrix_set(J, 0, 1, -cimag(sum_der));
    gsl_matrix_set(J, 1, 1,  creal(sum_der));

#if 0
    // DEBUG CODE
    printf("computed:\n");
    printf("%g %g\n", gsl_matrix_get(J, 0, 0), gsl_matrix_get(J, 0, 1));
    printf("%g %g\n", gsl_matrix_get(J, 1, 0), gsl_matrix_get(J, 1, 1));
    printf("\n");
#endif

    return GSL_SUCCESS;
}
#endif

static cmpl epsilon_start(const struct epsilon_sequence *seq) {
#if 0
    cmpl eps = 0;
    for (int i = 0; i < seq->length; i++) {
        eps += seq->terms[i].f * seq->terms[i].eps;
    }
    return eps;
#endif
    return 1;
}

static void epsilon_step_size(gsl_vector *step, const struct epsilon_sequence *seq, double factor) {
    double sr = 1, si = 1;
    for (int i = 0; i < seq->length; i++) {
        double re = factor * fabs(creal(seq->terms[i].eps));
        double im = factor * fabs(cimag(seq->terms[i].eps));
        sr = (re > sr ? re : sr);
        si = (im > si ? im : si);
    }
    gsl_vector_set(step, 0, sr);
    gsl_vector_set(step, 1, si);
}

void compute_epsilon_sequence_terms(const struct disp_bruggeman *d, struct epsilon_sequence *seq, double lam) {
    const cmpl n_base = n_value(d->disp_base, lam);
    const cmpl eps_base = n_base * n_base;
    double f_base = 1;
    for (int i = 1; i < seq->length; i++) {
        struct epsilon_fraction *t = &seq->terms[i];
        const cmpl n = n_value(d->components[i - 1].disp, lam);
        t->f = d->components[i - 1].fraction;
        t->eps = n * n;
        f_base -= t->f;
    }
    seq->terms[0].f = f_base;
    seq->terms[0].eps = eps_base;
}

static cmpl epsilon_iterative_refine(const struct epsilon_sequence *seq, cmpl eps0) {
    cmpl eps1;
    cmpl sum_fe = 0, sum_f1 = 0, sum_fesq = 0;
    for (int iteration = 0; iteration < 200; iteration++) {
        for (int i = 0; i < seq->length; i++) {
            const struct epsilon_fraction *t = &seq->terms[i];
            const cmpl den = t->eps + 2 * eps0;
            const cmpl den_square = den * den;
            sum_fe += t->f * t->eps / den;
            sum_f1 += t->f / den;
            sum_fesq += 3 * t->f * t->eps / den_square;
        }
        eps1 = sum_fe / sum_f1;
        cmpl delta_eps = eps1 - eps0;
        if (fabs(creal(delta_eps)) < 1e-6 && fabs(cimag(delta_eps)) < 1e-6) {
            break;
        }
        eps0 = eps1;
    }
    return eps1;
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

cmpl
bruggeman_n_value_deriv(const disp_t *disp, double lam, cmpl_vector *v)
{
    const struct disp_bruggeman *d = &disp->disp.bruggeman;

    const int terms_no = d->components_number + 1;
    struct epsilon_fraction terms[terms_no];
    struct epsilon_sequence seq[1] = {{terms_no, terms}};

    compute_epsilon_sequence_terms(d, seq, lam);

    const cmpl eps0 = epsilon_start(seq);
    double eps0_data[2] = {creal(eps0), cimag(eps0)};
    gsl_vector_view eps0_view = gsl_vector_view_array(eps0_data, 2);

    const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;
    gsl_multimin_fminimizer *s = gsl_multimin_fminimizer_alloc(T, 2);
    gsl_multimin_function f = {&bruggeman_epsilon_f, 2, seq};

    for (int min_iter = 0; min_iter < 20; min_iter++) {
        double eps_step_data[2];
        gsl_vector_view eps_step_view = gsl_vector_view_array(eps_step_data, 2);
        epsilon_step_size(&eps_step_view.vector, seq, 3.0 * (min_iter + 1));

        gsl_multimin_fminimizer_set(s, &f, &eps0_view.vector, &eps_step_view.vector);

        int iter, status;
        for (iter = 0; iter < 200; iter++) {
            status = gsl_multimin_fminimizer_iterate(s);
            if (status) {
                printf("ERROR: stopping iterations: %s\n", gsl_strerror(status));
                break;
            }
            double size = gsl_multimin_fminimizer_size(s);
            status = gsl_multimin_test_size(size, 1e-2);
            if (status != GSL_CONTINUE) {
                printf("%d,", iter);
                break;
            }
        }
        if (iter >= 200 || status) {
            printf("ERROR: algorithm fails to converge, %d %s\n", iter, gsl_strerror(status));
        } else {
            break;
        }
    }

    const cmpl eps_min = gsl_vector_get(s->x, 0) + I * gsl_vector_get(s->x, 1);
    gsl_multimin_fminimizer_free(s);

#if 0
    const cmpl residual = bruggeman_debug_verif(d, eps_min, lam);
    printf("%g,%g,%g,%g,%g,%g,%g,", lam,creal(eps0), cimag(eps0), creal(eps_min), cimag(eps_min), creal(residual), cimag(residual));
    cmpl n0 = csqrt(eps0);
    cmpl n = csqrt(eps_min);
    printf("%g,%g,%g,%g\n", creal(n0), cimag(n0), creal(n), cimag(n));
#endif

    cmpl eps = epsilon_iterative_refine(seq, eps_min);

    if (v != NULL) {
        cmpl sum_fesq = compute_sum_fesq(seq, eps);
        const cmpl deriv_factor = 1.0 / (2.0 * csqrt(eps));
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

    return csqrt(eps);

#if 0
    double f_base = 1.0;
    for (int i = 0; i < d->components_number; i++) {
        f_base -= d->components[i].fraction;
    }
    const cmpl n_base = n_value(d->disp_base, lam);
    const cmpl eps_base = n_base * n_base;
    cmpl eps0 = f_base * eps_base;

    for (int i = 0; i < d->components_number; i++) {
        eps0 += d->components[i].fraction * eps_c[i];
    }

    if (lam >= 248 && lam <= 252) {
        printf("%d %10g %10g\n", 0, creal(eps0), cimag(eps0));
    }

    cmpl eps1;
    cmpl sum_fe, sum_f1, sum_fesq;
    int final_iteration = 0;
    for (int iteration = 0; iteration < 200; iteration++) {
        const cmpl den = eps_base + 2 * eps0;
        sum_fe = f_base * eps_base / den;
        sum_f1 = f_base / den;
        const cmpl den_square = den * den;
        sum_fesq = 3 * f_base * eps_base / den_square;

        for (int i = 0; i < d->components_number; i++) {
            const double f = d->components[i].fraction;
            const cmpl den_i = eps_c[i] + 2 * eps0;
            const cmpl den_square_i = den_i * den_i;
            sum_fe += f * eps_c[i] / den_i;
            sum_f1 += f / den_i;
            sum_fesq += 3 * f * eps_c[i] / den_square_i;
        }

        if (final_iteration) {
            break;
        }

        eps1 = sum_fe / sum_f1;
#ifdef DEBUG_BRUGGEMAN
        if (lam >= 248 && lam <= 252) {
            printf("%d %10g %10g\n", iteration + 1, creal(eps1), cimag(eps1));
        }
#endif
        cmpl delta_eps = eps1 - eps0;
        if (fabs(creal(delta_eps)) < 1e-6 && fabs(cimag(delta_eps)) < 1e-6) {
            final_iteration = 1;
            if (!v) break;
        }

        eps0 = eps1;
    }

    if(v != NULL) {
        const cmpl resid_base = (eps_base - eps1) / (eps_base + 2 * eps1);
        const cmpl eps_der_base = resid_base / sum_fesq + eps1;
        const cmpl deriv_factor = 1.0 / (2.0 * csqrt(eps1));
        const cmpl n_der_base = deriv_factor * eps_der_base;

        for (int i = 0; i < d->components_number; i++) {
            const cmpl resid_i = (eps_c[i] - eps1) / (eps_c[i] + 2 * eps1);
            const cmpl eps_der_i = resid_i / sum_fesq + eps1;
            const cmpl n_der_i = deriv_factor * eps_der_i;
            cmpl_vector_set(v, i, n_der_i - n_der_base);
        }
    }

#ifdef DEBUG_BRUGGEMAN
    static int VERIF_DEBUG = 1;
    if (0 && VERIF_DEBUG) {
        cmpl sum_verif = bruggeman_debug_verif(d, eps1, lam);
        printf("sum: (%g,%g)\n", creal(sum_verif), cimag(sum_verif));
        bruggeman_debug_verif_deriv(disp, lam, &VERIF_DEBUG);
    }
#endif

    return csqrt(eps1);
#endif
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
