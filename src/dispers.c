#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <gsl/gsl_vector.h>

#include "dispers.h"
#include "common.h"
#include "cmpl.h"
#include "str.h"
#include "str-util.h"
#include "fit-params.h"
#include "dispers-library.h"

static void disp_info_init(struct disp_info *info, const char *name);
static void disp_info_free(struct disp_info *info);
static void disp_info_copy(struct disp_info *src, struct disp_info *dst);

static void
remove_filename_extension(str_t name)
{
    const char *base = CSTR(name);
    const char *p = strrchr(base, '.');
    if (p) {
        int len = p - base;
        str_trunc(name, len);
    }
}

disp_t *
load_nk_table(const char * filename, str_ptr *error_msg)
{
    disp_t * disp;

    disp = disp_table_new_from_nk_file(filename, error_msg);

    if(disp == NULL) {
        return NULL;
    }

    if (disp->info) {
        str_path_basename(disp->info->name, filename);
        remove_filename_extension(disp->info->name);
    }

    return disp;
}

disp_t *
load_mat_dispers(const char * filename, str_ptr *error_msg)
{
    return disp_sample_table_new_from_mat_file(filename, error_msg);
}


complex double
n_value(const disp_t *disp, double lam)
{
    assert(disp->dclass != NULL);
    return disp->dclass->n_value(disp, lam);
}

void
n_value_cpp(const disp_t *mat, double lam, double *nr, double *ni)
{
    cmpl n = n_value(mat, lam);
    *nr = creal(n);
    *ni = - cimag(n);
}

int
disp_get_number_of_params(const disp_t *d)
{
    assert(d->dclass != NULL);
    return d->dclass->fp_number(d);
}

double *
disp_map_param(disp_t *d, int index)
{
    assert(d->dclass != NULL);
    return d->dclass->map_param(d, index);
}

int
dispers_apply_param(disp_t *d, const fit_param_t *fp, double val)
{
    if(fp->id != PID_LAYER_N) {
        return 1;
    }
    assert(d->dclass != NULL);
    return d->dclass->apply_param(d, fp, val);
}

void
n_value_deriv(const disp_t *disp, cmpl_vector *der, double lambda)
{
    assert(disp->dclass != NULL);
    disp->dclass->n_value_deriv(disp, lambda, der);
}

void
get_model_param_deriv(const disp_t *disp, struct deriv_info *deriv_info,
                      const fit_param_t *fp, double lambda,
                      double *dnr, double *dni)
{
    cmpl val;

    assert(disp->dclass != NULL);

    if(! deriv_info->is_valid) {
        disp->dclass->n_value_deriv(disp, lambda, deriv_info->val);
        deriv_info->is_valid = 1;
    }

    val = cmpl_vector_get(deriv_info->val, fp->param_nb);

    *dnr = creal(val);
    *dni = cimag(val);
}

void
disp_free(disp_t *d)
{
    assert(d->dclass != NULL);
    d->dclass->free(d);
}

disp_t *
disp_new(enum disp_type tp)
{
    return disp_new_with_name(tp, NULL);
}

disp_t *
disp_new_with_name(enum disp_type tp, const char *name)
{
    disp_t *d = emalloc(sizeof(disp_t));
    d->dclass = disp_class_lookup(tp);
    d->type = tp;
    if(name) {
        d->info = emalloc(sizeof(struct disp_info));
        disp_info_init(d->info, name);
    } else {
        d->info = NULL;
    }
    return d;
}

disp_t *
disp_base_copy(const disp_t *src)
{
    disp_t *res = emalloc(sizeof(disp_t));
    memcpy(res, src, sizeof(disp_t));
    if (src->info) {
        res->info = emalloc(sizeof(struct disp_info));
        disp_info_copy(res->info, src->info);
    }
    return res;
}

void
disp_base_free(disp_t *d)
{
    disp_info_free(d->info);
    free(d);
}

int
disp_base_fp_number(const disp_t *src)
{
    return 0;
}

disp_t *
disp_copy(const disp_t *d)
{
    assert(d->dclass != NULL);
    return d->dclass->copy(d);
}

int
disp_integrity_check(disp_t *d)
{
    int tp;
    if(d == NULL) {
        return 1;
    }
    tp = d->type;
    return (tp <= DISP_UNSET || tp >= DISP_END_OF_TABLE);
}

int
disp_check_fit_param(disp_t *d, fit_param_t *fp)
{
    assert(d->dclass != NULL);

    int nb = d->dclass->fp_number(d);
    if(fp->id != PID_LAYER_N) {
        return 1;
    }
    if(d->dclass->disp_class_id != fp->model_id) {
        return 1;
    }
    if(fp->param_nb < 0 || fp->param_nb >= nb) {
        return 1;
    }
    return 0;
}

double
disp_get_param_value(const disp_t *d, const fit_param_t *fp)
{
    assert(d->dclass != NULL);
    return d->dclass->get_param_value(d, fp);
}

int
disp_samples_number(const disp_t *d)
{
    assert(d->dclass != NULL);
    if (d->dclass->samples_number) {
        return d->dclass->samples_number(d);
    }
    return 0;
}

double
disp_sample_wavelength(const disp_t *d, int index)
{
    assert(d->dclass != NULL);
    if (d->dclass->sample_wavelength) {
        return d->dclass->sample_wavelength(d, index);
    }
    return 0.0;
}

static int
write_library_id(writer_t *w, const char *id)
{
    writer_printf(w, "library \"%s\"", id);
    writer_newline(w);
    return 0;
}

int
disp_write(writer_t *w, const disp_t *d)
{
    assert(d->dclass != NULL);
    const char *lib_id = lib_disp_table_lookup(d);
    if (lib_id) {
        return write_library_id(w, lib_id);
    }
    return d->dclass->write(w, d);
}

static disp_t *
disp_read_header(lexer_t *l)
{
    if (lexer_ident(l)) return NULL;
    struct disp_class *dclass;
    void *iter;
    for (iter = disp_class_next(NULL); iter; iter = disp_class_next(iter)) {
        dclass = disp_class_from_iter(iter);
        if (strcmp(dclass->short_name, CSTR(l->store)) == 0) {
            break;
        }
    }
    if (iter == NULL) return NULL;
    if (lexer_string(l)) return NULL;
    return disp_new_with_name(dclass->disp_class_id, CSTR(l->store));
}

disp_t *
disp_read(lexer_t *l)
{
    if (lexer_check_ident(l, "library") == 0) {
        if (lexer_string(l)) return NULL;
        return lib_disp_table_get(CSTR(l->store));
    }
    disp_t *d = disp_read_header(l);
    if (!d) return NULL;
    if (d->dclass->read == NULL || d->dclass->read(l, d)) {
        disp_free(d);
        return NULL;
    }
    return d;
}

int
disp_is_tabular(const disp_t *d)
{
    return (d->type == DISP_TABLE || d->type == DISP_SAMPLE_TABLE);
}

const char *
disp_get_name(const disp_t *d)
{
    return d->info ? CSTR(d->info->name) : "";
}

void
disp_set_name(disp_t *d, const char *name)
{
    if (!d->info) {
        d->info = emalloc(sizeof(struct disp_info));
        disp_info_init(d->info, name);
    }
    str_copy_c(d->info->name, name);
}


void
disp_info_init(struct disp_info *info, const char *name)
{
    str_init_from_c(info->name, name);
    str_init(info->description, 15);
    info->wavelength_start = 0.0;
    info->wavelength_end   = 0.0;
}

void
disp_info_free(struct disp_info *info)
{
    str_free(info->name);
    str_free(info->description);
}

void
disp_info_copy(struct disp_info *src, struct disp_info *dst)
{
    str_init_from_str(src->name, dst->name);
    str_init_from_str(src->description, dst->description);
    src->wavelength_start = dst->wavelength_start;
    src->wavelength_end   = dst->wavelength_end;
}
