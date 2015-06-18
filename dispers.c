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

    str_path_basename(disp->name, filename);
    remove_filename_extension(disp->name);

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

int
disp_get_model_id(disp_t *d)
{
    assert(d->dclass != NULL);
    return d->dclass->model_id;
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
        str_init_from_c(d->name, name);
    } else {
        str_init(d->name, 16);
    }
    return d;
}

disp_t *
disp_base_copy(const disp_t *src)
{
    disp_t *res = emalloc(sizeof(disp_t));
    memcpy(res, src, sizeof(disp_t));
    str_init_from_str(res->name, src->name);
    return res;
}

void
disp_base_free(disp_t *d)
{
    str_free(d->name);
    free(d);
}

int
disp_base_decode_param_string(const char *param)
{
    return -1;
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
    if(d->dclass->model_id != fp->model_id) {
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
decode_fit_param(fit_param_t *fp, const str_t str)
{
    struct disp_class_node *disp_node;
    const char *s = CSTR(str);
    const char *snext;
    int layer;

    if(strcmp(s, "RMult") == 0) {
        fp->id = PID_FIRSTMUL;
        return 0;
    }

    snext = strchr(s, (int) ':');
    if(snext == NULL) {
        return 1;
    }
    if(strncmp(s, "Th:", 3) == 0) {
        char *tail;
        s = snext + 1;
        layer = strtol(s, & tail, 10);
        if(*tail != 0 || tail == s || layer < 0) {
            return 1;
        }
        set_thick_param(fp, layer);
        return 0;
    } else if(strncmp(s, "RI:", 3) == 0) {
        char *tail;
        s = snext + 1;
        layer = strtol(s, & tail, 10);
        if(*tail != ':' || tail == s || layer < 0) {
            return 1;
        }
        s = tail + 1;
        snext = strchr(s, (int) ':');
        if(snext == NULL) {
            return 1;
        }

        for(disp_node = disp_class_list; disp_node; disp_node = disp_node->next) {
            struct disp_class *dclass = disp_node->value;
            size_t dclass_len = strlen(dclass->short_id);

            if(strncmp(s, dclass->short_id, dclass_len) == 0) {
                int fp_number;
                const char *currs = s + dclass_len;
                if(currs[0] != ':') {
                    continue;
                }
                currs++;
                fp_number = dclass->decode_param_string(currs);
                if(fp_number < 0) {
                    return 1;
                }
                set_model_param(fp, layer, dclass->model_id, fp_number);
                return 0;
            }
        }
    }

    return 1;
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
    static const char *model[] = {"uniform-table", "table", "cauchy", "ho", "lookup", "bruggeman", "fb"};
    if (lexer_ident(l)) return NULL;
    enum disp_type model_id;
    for (model_id = DISP_TABLE; model_id < DISP_END_OF_TABLE; model_id++) {
        if (strcmp(model[model_id - 1], CSTR(l->store)) == 0) {
            break;
        }
    }
    if (model_id == DISP_END_OF_TABLE) return NULL;
    if (lexer_string(l)) return NULL;
    return disp_new_with_name(model_id, CSTR(l->store));
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
