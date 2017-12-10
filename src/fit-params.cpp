#include <stdio.h>
#include <string.h>

#include "fit-params.h"
#include "dispers.h"
#include "acquisition.h"

static void get_disp_param_name(const fit_param_t *fp, str_ptr buf);


void
set_model_param(fit_param_t *fpres, int lyr, enum disp_type model_id,
                int param_nb)
{
    fpres->id = PID_LAYER_N;
    fpres->layer_nb = lyr;
    fpres->model_id = model_id;
    fpres->param_nb = param_nb;
}

void
set_thick_param(fit_param_t *fpres, int lyr)
{
    fpres->id = PID_THICKNESS;
    fpres->layer_nb = lyr;
    fpres->model_id = DISP_UNSET;
    fpres->param_nb = 0;
}

void
get_param_name(const fit_param_t *fp, str_t name)
{
    if (fp->id >= PID_ACQUISITION_PARAMETER) {
        acquisition_parameter_to_string(name, fp->id);
    } else if (fp->id == PID_THICKNESS) {
        str_printf(name, "T%i", fp->layer_nb);
    } else if (fp->id == PID_LAYER_N) {
        get_disp_param_name(fp, name);
    } else {
        str_printf(name, "###");
    }
}

void
get_full_param_name(const fit_param_t *fp, str_t name)
{
    if (fp->id >= PID_ACQUISITION_PARAMETER) {
        acquisition_parameter_to_string(name, fp->id);
    } else if (fp->id == PID_THICKNESS) {
        str_printf(name, "T%i", fp->layer_nb);
    } else if (fp->id == PID_LAYER_N) {
        str_t dname;
        str_init(dname, 15);
        get_disp_param_name(fp, dname);
        str_printf(name, "Layer%d / %s", fp->layer_nb, CSTR(dname));
        str_free(dname);
    } else {
        str_printf(name, "###");
    }
}

void
get_disp_param_name(const fit_param_t *fp, str_ptr buf)
{
    struct disp_class *dclass;
    void *iter;

    if(fp->id != PID_LAYER_N) {
        str_copy_c(buf, "###");
        return;
    }

    for(iter = disp_class_next(nullptr); iter; iter =  disp_class_next(iter)) {
        dclass = disp_class_from_iter(iter);
        if(dclass->disp_class_id == fp->model_id) {
            break;
        }
    }

    if(iter) {
        dclass->encode_param(buf, fp);
    } else {
        str_copy_c(buf, "###");
    }
}

fit_parameters *fit_parameters_new() {
    return new fit_parameters();
}

void fit_parameters_free(fit_parameters *s) {
    delete s;
}

void fit_parameters_clear(fit_parameters *s) {
    s->clear();
}

fit_parameters *fit_parameters_copy(const fit_parameters *fps) {
    return new fit_parameters(*fps);
}

int fit_parameters_add(fit_parameters *lst, fit_param_t const * fp) {
    lst->add(*fp);
    return lst->number;
}

void
fit_parameters_remove(fit_parameters *lst, int index) {
    lst->erase(index);
}

seeds_list *seed_list_new() {
    return new seeds_list();
}

void seed_list_free(seeds_list *s) {
    delete s;
}

void seed_list_add_simple(seeds_list *s, double v) {
    s->add(seed_t{SEED_SIMPLE, v});   
}

void seed_list_add(seeds_list *s, const seed_t *v) {
    s->add(*v);
}

void seed_list_remove(seeds_list *lst, int index) {
    lst->erase(index);
}

static int
seed_write(writer_t *w, const seed_t *s)
{
    if (s->type == SEED_SIMPLE) {
        writer_printf(w, "value %g", s->seed);
    } else if (s->type == SEED_RANGE) {
        writer_printf(w, "range %g %g", s->seed, s->delta);
    } else {
        writer_printf(w, "undef");
    }
    return 0;
}

static int
seed_read(lexer_t *l, seed_t *seed)
{
    if (lexer_ident(l)) return 1;
    if (strcmp(CSTR(l->store), "value") == 0) {
        seed->type = SEED_SIMPLE;
        if (lexer_number(l, &seed->seed)) return 1;
    } else if (strcmp(CSTR(l->store), "range") == 0) {
        seed->type = SEED_RANGE;
        if (lexer_number(l, &seed->seed)) return 1;
        if (lexer_number(l, &seed->delta)) return 1;
    } else if (strcmp(CSTR(l->store), "undef") == 0) {
        seed->type = SEED_UNDEF;
    } else {
        return 1;
    }
    return 0;
}

int
seed_list_write(writer_t *w, const seeds_list *s)
{
    writer_printf(w, "seed-list %d", s->number);
    writer_newline_enter(w);
    int i;
    for (i = 0; i < s->number; i++) {
        if (i > 0) {
            writer_newline(w);
        }
        seed_write(w, &s->at(i));
    }
    writer_newline_exit(w);
    return 1;
}

seeds_list *
seed_list_read(lexer_t *l)
{
    int i, nb;
    seeds_list *s = seed_list_new();
    if (lexer_check_ident(l, "seed-list")) goto seeds_exit;
    if (lexer_integer(l, &nb)) goto seeds_exit;
    for (i = 0; i < nb; i++) {
        seed_t seed[1];
        if (seed_read(l, seed)) goto seeds_exit;
        seed_list_add(s, seed);
    }
    return s;
seeds_exit:
    seed_list_free(s);
    return nullptr;
}

struct strategy *
strategy_new() {
    struct strategy *r;

    r = (struct strategy *) emalloc(sizeof(struct strategy));

    r->parameters = fit_parameters_new();
    r->seeds = seed_list_new();

    return r;
}

void
strategy_free(struct strategy *s)
{
    fit_parameters_free(s->parameters);
    seed_list_free(s->seeds);
    free(s);
}

int fit_parameters_are_RI_fixed(fit_parameters *f) {
    int j;
    for(j = 0; j < f->number; j++) {
        if(f->at(j).id == PID_LAYER_N) {
            break;
        }
    }
    return (j >= f->number);
}

int
fit_parameters_contains_acquisition_parameters(fit_parameters *f)
{
    for(int j = 0; j < f->number; j++) {
        const int id = f->at(j).id;
        if(id >= PID_ACQUISITION_PARAMETER || id < PID_INVALID) {
            return 1;
        }
    }
    return 0;
}

int
fit_param_compare(const fit_param_t *a, const fit_param_t *b)
{
    if (a->id != b->id) return 1;
    if (a->layer_nb != b->layer_nb) return 1;
    if (a->id == PID_LAYER_N) {
        if (a->model_id != b->model_id) return 1;
        if (a->param_nb != b->param_nb) return 1;
    }
    return 0;
}

int
fit_parameters_find(const fit_parameters *lst, const fit_param_t *fp)
{
    int j;
    for(j = 0; j < lst->number; j++) {
        const fit_param_t *lfp = &lst->at(j);
        if (fit_param_compare(lfp, fp) == 0) {
            return j;
        }
    }
    return -1;
}

static void
fix_delete_layer(fit_parameters *lst, int index)
{
    for (int i = 0; i < lst->number; i++) {
        fit_param_t *fp = &lst->at(i);
        if (fp->id < PID_ACQUISITION_PARAMETER && fp->layer_nb >= index) {
            if (fp->layer_nb > index) {
                fp->layer_nb --;
            } else {
                fp->id = PID_INVALID;
            }
        }
    }
}

static void
fix_insert_layer(fit_parameters *lst, int index)
{
    for (int i = 0; i < lst->number; i++) {
        fit_param_t *fp = &lst->at(i);
        if (fp->id < PID_ACQUISITION_PARAMETER && fp->layer_nb >= index) {
            fp->layer_nb ++;
        }
    }
}

void
fit_parameters_fix_layer_shift(fit_parameters *lst, struct shift_info shift)
{
    if (shift.event == SHIFT_DELETE_LAYER) {
        fix_delete_layer(lst, shift.index);
    } else if (shift.event == SHIFT_INSERT_LAYER) {
        fix_insert_layer(lst, shift.index);
    }
}

static int
fit_param_write(writer_t *w, const fit_param_t *fp)
{
    int id = fp->id;
    const char *fp_name;
    if (id == PID_THICKNESS) {
        fp_name = "thickness";
    } else if (id == PID_LAYER_N) {
        fp_name = "n";
    } else if (id >= PID_ACQUISITION_PARAMETER && id < PID_INVALID) {
        fp_name = acquisition_parameter_name(id);
    } else {
        fp_name = "invalid";
    }
    writer_printf(w, "%s", fp_name);
    if (id < PID_ACQUISITION_PARAMETER) {
        writer_printf(w, " %d", fp->layer_nb);
        if (id == PID_LAYER_N) {
            struct disp_class *dclass = disp_class_lookup(fp->model_id);
            writer_printf(w, " %s %d", dclass ? dclass->short_name : "unknown", fp->param_nb);
        }
    }
    return 0;
}

static int
fit_param_read(lexer_t *l, fit_param_t *fp)
{
    if (lexer_ident(l)) return 1;
    if (strcmp(CSTR(l->store), "thickness") == 0) {
        fp->id = PID_THICKNESS;
        if (lexer_integer(l, &fp->layer_nb)) return 1;
    } else if (strcmp(CSTR(l->store), "n") == 0) {
        fp->id = PID_LAYER_N;
        if (lexer_integer(l, &fp->layer_nb)) return 1;
        if (lexer_ident(l)) return 1;
        void *iter;
        for (iter = disp_class_next(nullptr); iter; iter = disp_class_next(iter)) {
            struct disp_class *dclass = disp_class_from_iter(iter);
            if (strcmp(dclass->short_name, CSTR(l->store)) == 0) {
                fp->model_id = dclass->disp_class_id;
                break;
            }
        }
        if (iter == nullptr) return 1;
        if (lexer_integer(l, &fp->param_nb)) return 1;
    } else if (strcmp(CSTR(l->store), "rmult") == 0) {
        fp->id = PID_FIRSTMUL;
    } else if (strcmp(CSTR(l->store), "aoi") == 0) {
        fp->id = PID_AOI;
    } else if (strcmp(CSTR(l->store), "analyzer") == 0) {
        fp->id = PID_ANALYZER;
    }
    return 0;
}

int
fit_parameters_write(writer_t *w, const fit_parameters *s)
{
    writer_printf(w, "fit-parameters %d", s->number);
    writer_newline_enter(w);
    int i;
    for (i = 0; i < s->number; i++) {
        if (i > 0) {
            writer_newline(w);
        }
        fit_param_write(w, &s->at(i));
    }
    writer_newline_exit(w);
    return 1;
}

fit_parameters *
fit_parameters_read(lexer_t *l)
{
    int nb;
    fit_parameters *fps = fit_parameters_new();
    if (lexer_check_ident(l, "fit-parameters")) goto params_exit;
    if (lexer_integer(l, &nb)) goto params_exit;
    int i;
    for (i = 0; i < nb; i++) {
        fit_param_t fp[1];
        if (fit_param_read(l, fp)) goto params_exit;
        fit_parameters_add(fps, fp);
    }
    return fps;
params_exit:
    fit_parameters_free(fps);
    return nullptr;
}
