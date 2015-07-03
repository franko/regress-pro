#include <string.h>
#include <assert.h>
#include "stack.h"
#include "str.h"

void
stack_init_raw(stack_t *s, size_t nb_init)
{
    s->nb = 0;
    s->nb_alloc = nb_init;
    s->disp = emalloc(nb_init * sizeof(void *));
    /* For thicknesses we allocate more space of what is actually needed. */
    s->thickness = emalloc(nb_init * sizeof(double));
}

void
stack_init(stack_t *s)
{
    stack_init_raw(s, 8);
}

static void
ensure_size(stack_t *s, int req_size)
{
    if((size_t)req_size > s->nb_alloc)
    {
        size_t nsz;
        s->nb_alloc *= 2;
        nsz = s->nb_alloc;
        s->disp = erealloc(s->disp, nsz * sizeof(void *));
        /* For thicknesses we allocate more space of what is actually needed. */
        s->thickness = erealloc(s->thickness,  nsz * sizeof(double));
    }
}

void
stack_add_layer(stack_t *s, disp_t *lyr, double th)
{
    size_t idx;

    ensure_size(s, s->nb + 1);

    idx = s->nb;

    s->disp[idx] = lyr;
    if(idx > 0) {
        s->thickness[idx - 1] = th;
    }

    s->nb ++;
}

void
stack_insert_layer(stack_t *s, int pos, disp_t *lyr, double th)
{
    int i;
    ensure_size(s, s->nb + 1);
    for (i = s->nb - 1; i >= pos; i--) {
        s->disp[i + 1] = s->disp[i];
    }
    s->disp[pos] = lyr;
    int lpos = pos > 0 ? pos - 1 : 0;
    for (i = s->nb - 3; i >= lpos; i--) {
        s->thickness[i+1] = s->thickness[i];
    }
    if (pos > 0) {
        s->thickness[pos-1] = th;
    } else {
        s->thickness[0] = 0.0;
    }
    s->nb++;
}

void
stack_delete_layer(stack_t *s, int pos)
{
    int i;
    disp_free(s->disp[pos]);
    for (i = pos; i + 1 < s->nb; i++) {
        s->disp[i] = s->disp[i+1];
    }
    int lpos = pos > 0 ? pos - 1 : 0;
    for (i = lpos; i + 1 < s->nb - 2; i++) {
        s->thickness[i] = s->thickness[i+1];
    }
    s->nb--;
}

stack_t *
stack_copy(const stack_t *s)
{
    int j;
    stack_t *r = emalloc(sizeof(stack_t));
    memcpy(r, s, sizeof(stack_t));

    r->nb_alloc = s->nb;

    r->thickness = emalloc(r->nb_alloc * sizeof(double));
    memcpy(r->thickness, s->thickness, r->nb_alloc * sizeof(double));

    r->disp = emalloc(r->nb_alloc * sizeof(disp_t *));
    for(j = 0; j < r->nb; j++) {
        r->disp[j] = disp_copy(s->disp[j]);
    }

    return r;
}

int
stack_apply_param(stack_t *s, const fit_param_t *fp, double val)
{
    int nlyr;
    int res = 0;

    switch(fp->id) {
    case PID_THICKNESS:
        nlyr = fp->layer_nb;
        s->thickness[ nlyr - 1 ] = val;
        break;
    case PID_LAYER_N:
        nlyr = fp->layer_nb;
        dispers_apply_param(s->disp[nlyr], fp, val);
        break;
    default:
        res = 1;
    }

    return res;
}

const double *
stack_get_ths_list(const stack_t *s)
{
    return s->thickness;
}

void
stack_get_ns_list(stack_t *s, cmpl *ns, double lambda)
{
    int j;
    for(j = 0; j < s->nb; j++) {
        ns[j] = n_value(s->disp[j], lambda);
    }
}

void
stack_free(stack_t *s)
{
    int j;
    free(s->thickness);
    for(j = 0; j < s->nb; j++) {
        disp_free(s->disp[j]);
    }
    free(s->disp);
    free(s);
}

void
stack_get_all_parameters(stack_t *stack, struct fit_parameters *fps)
{
    int n_layers = stack->nb - 2;
    fit_param_t fp[1];
    int j;

    fp->id = PID_THICKNESS;
    for(j = 1; j < n_layers + 1; j++) {
        fp->layer_nb = j;
        fit_parameters_add(fps, fp);
    }

    fp->id = PID_LAYER_N;
    for(j = 1; j < n_layers + 2; j++) {
        disp_t *d = stack->disp[j];
        int k, np = disp_get_number_of_params(d);
        fp->layer_nb = j;
        fp->model_id = d->type;
        for(k = 0; k < np; k++) {
            fp->param_nb = k;
            fit_parameters_add(fps, fp);
        }
    }
}

double
stack_get_parameter_value(const stack_t *st, const fit_param_t *fp)
{
    if(fp->id == PID_THICKNESS) {
        int layer_nb = fp->layer_nb;
        assert(layer_nb > 0 && layer_nb < st->nb - 1);
        return st->thickness[layer_nb-1];
    } else if(fp->id == PID_LAYER_N) {
        int layer_nb = fp->layer_nb;
        const disp_t *d = st->disp[layer_nb];
        assert(layer_nb > 0 && layer_nb <= st->nb - 1);
        return disp_get_param_value(d, fp);
    }
    return 0.0;
}

int
stack_write(writer_t *w, const stack_t *s)
{
    writer_printf(w, "stack %d", s->nb);
    writer_newline_enter(w);
    int i;
    for (i = 1; i < s->nb - 1; i++) {
        if (i > 1) {
            writer_printf(w, " ");
        }
        writer_printf(w, "%g", s->thickness[i-1]);
    }
    writer_newline(w);
    for (i = 0; i < s->nb; i++) {
        disp_write(w, s->disp[i]);
    }
    writer_indent(w, -1);
    return 0;
}

stack_t *
stack_read(lexer_t *l)
{
    int i, nb;
    if (lexer_check_ident(l, "stack")) return NULL;
    if (lexer_integer(l, &nb)) return NULL;
    stack_t *s = emalloc(sizeof(stack_t));
    stack_init_raw(s, nb);
    for (i = 1; i < nb - 1; i++) {
        if (lexer_number(l, s->thickness + (i - 1))) goto stack_exit;
    }
    for (i = 0; i < nb; i++, s->nb++) {
        s->disp[i] = disp_read(l);
        if (!s->disp[i]) goto stack_exit;
    }
    return s;
stack_exit:
    stack_free(s);
    return 0;
}
