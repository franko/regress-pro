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
static void disp_info_init_and_copy(struct disp_info *src, struct disp_info *dst);

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

    if(disp == nullptr) {
        return nullptr;
    }

    str_path_basename(disp->info->name, filename);
    remove_filename_extension(disp->info->name);

    return disp;
}

disp_t *
load_mat_dispers(const char * filename, str_ptr *error_msg)
{
    return disp_sample_table_new_from_mat_file(filename, error_msg);
}


cmpl
n_value(const disp_t *disp, double lam)
{
    assert(disp->dclass != nullptr);
    return disp->dclass->n_value(disp, lam);
}

void
n_value_cpp(const disp_t *mat, double lam, double *nr, double *ni)
{
    cmpl n = n_value(mat, lam);
    *nr = std::real(n);
    *ni = -std::imag(n);
}

int
disp_get_number_of_params(const disp_t *d)
{
    assert(d->dclass != nullptr);
    return d->dclass->fp_number(d);
}

double *
disp_map_param(disp_t *d, int index)
{
    assert(d->dclass != nullptr);
    return d->dclass->map_param(d, index);
}

int
dispers_apply_param(disp_t *d, const fit_param_t *fp, double val)
{
    if(fp->id != PID_LAYER_N) {
        return 1;
    }
    assert(d->dclass != nullptr);
    return d->dclass->apply_param(d, fp, val);
}

void
n_value_deriv(const disp_t *disp, cmpl_vector *der, double lambda)
{
    assert(disp->dclass != nullptr);
    disp->dclass->n_value_deriv(disp, lambda, der);
}

void
get_model_param_deriv(const disp_t *disp, struct deriv_info *deriv_info,
                      const fit_param_t *fp, double lambda,
                      double *dnr, double *dni)
{
    assert(disp->dclass != nullptr);

    if(! deriv_info->is_valid) {
        disp->dclass->n_value_deriv(disp, lambda, deriv_info->val);
        deriv_info->is_valid = 1;
    }

    cmpl val = deriv_info->val->at(fp->param_nb);

    *dnr = std::real(val);
    *dni = std::imag(val);
}

void
disp_free(disp_t *d)
{
    assert(d->dclass != nullptr);
    d->dclass->free(d);
}

disp_t *
disp_new(enum disp_type tp)
{
    return disp_new_with_name(tp, "");
}

disp_t *
disp_new_with_name(enum disp_type tp, const char *name)
{
    assert(name != nullptr);
    disp_t *d = (disp_t *) emalloc(sizeof(disp_t));
    d->dclass = disp_class_lookup(tp);
    d->type = tp;
    d->info = (struct disp_info *) emalloc(sizeof(struct disp_info));
    disp_info_init(d->info, name);
    return d;
}

disp_t *
disp_base_copy(const disp_t *src)
{
    disp_t *res = (disp_t *) emalloc(sizeof(disp_t));
    memcpy(res, src, sizeof(disp_t));
    res->info = (struct disp_info *) emalloc(sizeof(struct disp_info));
    disp_info_init_and_copy(res->info, src->info);
    return res;
}

void
disp_base_free(disp_t *d)
{
    disp_info_free(d->info);
    free(d->info);
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
    assert(d->dclass != nullptr);
    return d->dclass->copy(d);
}

int
disp_integrity_check(disp_t *d)
{
    int tp;
    if(d == nullptr) {
        return 1;
    }
    tp = d->type;
    return (tp <= DISP_UNSET || tp >= DISP_END_OF_TABLE);
}

int
disp_check_fit_param(disp_t *d, fit_param_t *fp)
{
    assert(d->dclass != nullptr);

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
    assert(d->dclass != nullptr);
    return d->dclass->get_param_value(d, fp);
}

int
disp_samples_number(const disp_t *d)
{
    assert(d->dclass != nullptr);
    if (d->dclass->samples_number) {
        return d->dclass->samples_number(d);
    }
    return 0;
}

double
disp_sample_wavelength(const disp_t *d, int index)
{
    assert(d->dclass != nullptr);
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
    assert(d->dclass != nullptr);
    const char *lib_id = lib_disp_table_lookup(d);
    if (lib_id) {
        return write_library_id(w, lib_id);
    }
    disp_base_write(w, d->dclass->short_name, d);
    return d->dclass->write(w, d);
}

int
disp_base_write(writer_t *w, const char *tag, const disp_t *d)
{
    str_t quoted;
    str_init(quoted, 127);
    writer_quote_string(quoted, disp_get_name(d));
    writer_printf(w, "%s %s", tag, CSTR(quoted));
    writer_newline_enter(w);
    if (!str_is_null(d->info->description)) {
        writer_quote_string(quoted, CSTR(d->info->description));
        writer_put_string(w, "description ");
        writer_put_string(w, CSTR(quoted));
        writer_newline(w);
    }
    if (d->info->modifications_stamp) {
        writer_quote_string(quoted, CSTR(d->info->modifications_stamp));
        writer_put_string(w, "annotation ");
        writer_put_string(w, CSTR(quoted));
        writer_newline(w);
    }
    if (DISP_VALID_RANGE(d->info->wavelength_start, d->info->wavelength_end)) {
        writer_printf(w, "range %g %g", d->info->wavelength_start, d->info->wavelength_end);
        writer_newline(w);
    }
    str_free(quoted);
    return 0;
}

static disp_t *
disp_read_header(lexer_t *l)
{
    disp_t *new_disp = nullptr;
    if (lexer_ident(l)) return nullptr; // Read the dispersion tag.
    struct disp_class *dclass;
    void *iter;
    for (iter = disp_class_next(nullptr); iter; iter = disp_class_next(iter)) {
        dclass = disp_class_from_iter(iter);
        // Look for a dispersion class that match the tag.
        if (strcmp(dclass->short_name, CSTR(l->store)) == 0) {
            break;
        }
    }
    if (iter == nullptr) return nullptr;
    if (lexer_string(l)) return nullptr; // Read the dispersion's name.
    str_t name;
    str_init_from_str(name, l->store);

    str_ptr description = nullptr;
    str_ptr modifications_stamp = nullptr;
    if (lexer_check_ident(l, "description") == 0) {
        if (lexer_string(l)) goto read_exit_1;
        description = str_new();
        str_copy(description, l->store);
    }
    if (lexer_check_ident(l, "annotation") == 0) {
        if (lexer_string(l)) goto read_exit_2;
        modifications_stamp = str_new();
        str_copy(modifications_stamp, l->store);
    }
    {
        double wavelength_start = 0.0, wavelength_end = 0.0;
        if (lexer_check_ident(l, "range") == 0) {
            if (lexer_number(l, &wavelength_start)) goto read_exit_3;
            if (lexer_number(l, &wavelength_end  )) goto read_exit_3;
        }
        new_disp = disp_new_with_name(dclass->disp_class_id, CSTR(name));
        if (description) {
            str_copy(new_disp->info->description, description);
        }
        assert(new_disp->info->modifications_stamp == nullptr);
        if (modifications_stamp) {
            new_disp->info->modifications_stamp = modifications_stamp;
            modifications_stamp = nullptr;
        }
        if (DISP_VALID_RANGE(wavelength_start, wavelength_end)) {
            new_disp->info->wavelength_start = wavelength_start;
            new_disp->info->wavelength_end   = wavelength_end;
        }
    }
read_exit_3:
    if (modifications_stamp) {
        str_free(modifications_stamp);
        free(modifications_stamp);
    }
read_exit_2:
    if (description) {
        str_free(description);
        free(description);
    }
read_exit_1:
    str_free(name);
    return new_disp;
}

disp_t *
disp_read(lexer_t *l)
{
    if (lexer_check_ident(l, "library") == 0) {
        if (lexer_string(l)) return nullptr;
        return lib_disp_table_get(CSTR(l->store));
    }
    disp_t *d = disp_read_header(l);
    if (!d) return nullptr;
    if (d->dclass->read == nullptr || d->dclass->read(l, d)) {
        disp_free(d);
        return nullptr;
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
    return CSTR(d->info->name);
}

void
disp_set_name(disp_t *d, const char *name)
{
    str_copy_c(d->info->name, name);
}

void
disp_set_info_wavelength(disp_t *d, double wavelength_start, double wavelength_end)
{
    d->info->wavelength_start = wavelength_start;
    d->info->wavelength_end   = wavelength_end;
}

void
disp_get_wavelength_range(const disp_t *d, double *wavelength_start, double *wavelength_end, int *samples_number)
{
    if (DISP_VALID_RANGE(d->info->wavelength_start, d->info->wavelength_end)) {
        *wavelength_start = d->info->wavelength_start;
        *wavelength_end   = d->info->wavelength_end;
        *samples_number   = 512;
    } else if (disp_is_tabular(d)) {
        int n = disp_samples_number(d);
        *wavelength_start = disp_sample_wavelength(d, 0);
        *wavelength_end   = disp_sample_wavelength(d, n - 1);
        *samples_number   = 512;
    } else {
        *wavelength_start = 250.0;
        *wavelength_end   = 750.0;
        *samples_number   = 251;
    }
}

void
disp_info_init(struct disp_info *info, const char *name)
{
    str_init_from_c(info->name, name);
    str_init(info->description, 15);
    info->modifications_stamp = nullptr;
    info->wavelength_start = 0.0;
    info->wavelength_end   = 0.0;
}

void
disp_info_free(struct disp_info *info)
{
    str_free(info->name);
    str_free(info->description);
    if (info->modifications_stamp) {
        str_free(info->modifications_stamp);
        free(info->modifications_stamp);
    }
}

static void disp_info_clear_modifications(struct disp_info *info) {
    if (info->modifications_stamp) {
        str_free(info->modifications_stamp);
        free(info->modifications_stamp);
    }
    info->modifications_stamp = nullptr;
}

void
disp_info_init_and_copy(struct disp_info *src, struct disp_info *dst)
{
    str_init_from_str(src->name, dst->name);
    str_init_from_str(src->description, dst->description);
    if (dst->modifications_stamp) {
        src->modifications_stamp = str_new();
        str_copy(src->modifications_stamp, dst->modifications_stamp);
    } else {
        src->modifications_stamp = nullptr;
    }
    src->wavelength_start = dst->wavelength_start;
    src->wavelength_end   = dst->wavelength_end;
}

void
disp_info_copy(struct disp_info *src, struct disp_info *dst)
{
    str_copy(src->name, dst->name);
    str_copy(src->description, dst->description);
    if (dst->modifications_stamp) {
        if (!src->modifications_stamp) {
            src->modifications_stamp = str_new();
        }
        str_copy(src->modifications_stamp, dst->modifications_stamp);
    } else {
        disp_info_clear_modifications(src);
    }
    src->wavelength_start = dst->wavelength_start;
    src->wavelength_end   = dst->wavelength_end;
}

void
disp_set_modifications_flag(const disp_t *d, const char *text)
{
    if (!d->info->modifications_stamp) {
        d->info->modifications_stamp = str_new();
    }
    str_copy_c(d->info->modifications_stamp, text);
}
