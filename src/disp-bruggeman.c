
#include <assert.h>
#include <string.h>

// DEBUG ONLY
#include <gsl/gsl_deriv.h>

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

cmpl
bruggeman_n_value_deriv(const disp_t *disp, double lam, cmpl_vector *v)
{
    const struct disp_bruggeman *d = &disp->disp.bruggeman;
    cmpl eps_c[d->components_number];
    for (int i = 0; i < d->components_number; i++) {
        const cmpl n = n_value(d->components[i].disp, lam);
        eps_c[i] = n * n;
    }

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
            printf("final value: (%10g, %10g)\n", creal(eps1), cimag(eps1));
            printf("number of iterations: %d\n", iteration);
            break;
        }

        eps1 = sum_fe / sum_f1;
        if (iteration == 0) {
            printf("first value: (%10g, %10g)\n", creal(eps1), cimag(eps1));
        }
        cmpl delta_eps = eps1 - eps0;
        if (fabs(creal(delta_eps)) < 1e-6 && fabs(cimag(delta_eps)) < 1e-6) {
            final_iteration = 1;
            // if (!v) break;
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

    static int VERIF_DEBUG = 1;
    if (VERIF_DEBUG) {
        cmpl sum_verif = bruggeman_debug_verif(d, eps1, lam);
        printf("sum: (%g,%g)\n", creal(sum_verif), cimag(sum_verif));
        bruggeman_debug_verif_deriv(disp, lam, &VERIF_DEBUG);
    }

    return csqrt(eps1);
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
