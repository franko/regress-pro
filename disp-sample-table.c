#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dispers.h"
#include "disp-sample-table.h"
#include "error-messages.h"

static void     disp_sample_table_free(disp_t *d);
static disp_t * disp_sample_table_copy(const disp_t *d);

static cmpl disp_sample_table_n_value(const disp_t *disp, double lam);
static int disp_sample_table_write(writer_t *w, const disp_t *_d);
static int disp_sample_table_read(lexer_t *l, disp_t *d);

struct disp_class disp_sample_table_class = {
    .disp_class_id       = DISP_SAMPLE_TABLE,
    .model_id            = MODEL_NONE,

    .short_id            = "SampleTable",
    .full_id             = "SampleTableDispersion",

    .free                = disp_sample_table_free,
    .copy                = disp_sample_table_copy,

    .n_value             = disp_sample_table_n_value,
    .fp_number           = disp_base_fp_number,
    .n_value_deriv       = NULL,
    .apply_param         = NULL,
    .get_param_value     = NULL,

    .decode_param_string = disp_base_decode_param_string,
    .encode_param        = NULL,
    .write               = disp_sample_table_write,
    .read                = disp_sample_table_read,
};

static void
disp_sample_table_init(struct disp_sample_table dt[], int nb)
{
    dt->nb = nb;
    dt->table_ref = data_table_new(nb, 3 /* columns */);
}

static void
disp_sample_table_clear(struct disp_sample_table *dt)
{
    dt->nb = 0;
}

void
disp_sample_table_free(disp_t *d)
{
    struct disp_sample_table *dt = &d->disp.sample_table;
    if(dt->nb > 0) {
        data_table_unref(dt->table_ref);
    }
    disp_base_free(d);
}

disp_t *
disp_sample_table_copy(const disp_t *src)
{
    disp_t *res = disp_base_copy(src);
    struct disp_sample_table *dt = &res->disp.sample_table;
    if(dt->nb > 0) {
        data_table_ref(dt->table_ref);
    }
    return res;
}

cmpl
disp_sample_table_n_value(const disp_t *disp, double _lam)
{
    const struct disp_sample_table *dt = & disp->disp.sample_table;
    struct data_table *t = dt->table_ref;
    float lam = _lam;
    int j, nb = dt->nb;

    for(j = 0; j < nb-1; j++) {
        double jlam = data_table_get(t, j+1, 0);
        if(jlam >= lam) {
            break;
        }
    }

    if(j >= nb-1) {
        return data_table_get(t, j, 1) + data_table_get(t, j, 2) * I;
    }

    float lam1 = data_table_get(t, j, 0), lam2 = data_table_get(t, j+1, 0);
    int j1 = j, j2 = j+1;

    float a = (lam - lam1) / (lam2 - lam1);
    float complex n1 = data_table_get(t, j1, 1) - data_table_get(t, j1, 2) * I;
    float complex n2 = data_table_get(t, j2, 1) - data_table_get(t, j2, 2) * I;

    return n1 + (n2 - n1) * a;
}

disp_t *
disp_sample_table_new_from_mat_file(const char * filename)
{
    FILE * f;
    str_t row;
    disp_t *disp = NULL;
    enum disp_type dtype;
    int convert_ev = 0;
    int provide_diel_k = 0;

    f = fopen(filename, "r");

    if(f == NULL) {
        notify_error_msg(LOADING_FILE_ERROR, "Cannot open %s", filename);
        return NULL;
    }

    str_init(row, 64);

    str_getline(row, f);
    str_getline(row, f);

    if(strncasecmp(CSTR(row), "CAUCHY", 6) == 0) {
        dtype = DISP_CAUCHY;
    } else if(strncasecmp(CSTR(row), "ev", 2) == 0) {
        convert_ev = 1;
    } else if(strncasecmp(CSTR(row), "nm", 2) != 0) {
        notify_error_msg(LOADING_FILE_ERROR, "Invalide MAT format: %s",
                         filename);
        goto close_exit;
    }

    str_getline(row, f);
    if(strncasecmp(CSTR(row), "nk", 2) == 0) {
        dtype = DISP_SAMPLE_TABLE;
    } else if(strncasecmp(CSTR(row), "e1e2", 4) == 0) {
        dtype = DISP_SAMPLE_TABLE;
        provide_diel_k = 1;
    } else {
        notify_error_msg(LOADING_FILE_ERROR, "Invalide MAT format: %s",
                         filename);
        goto close_exit;
    }

    switch(dtype) {
    case DISP_SAMPLE_TABLE: {
        struct disp_sample_table *dt;
        struct data_table *table;
        long start_pos = ftell(f);
        int j, lines;

        disp = disp_new(DISP_SAMPLE_TABLE);
        dt = & disp->disp.sample_table;
        disp_sample_table_clear(dt);

        start_pos = ftell(f);

        for(lines = 0; ;) {
            float xd[3];
            int read_status = fscanf(f, "%f %f %f\n", xd, xd+1, xd+2);
            if(read_status == 3) {
                lines ++;
            }
            if(read_status == EOF) {
                break;
            }
        }

        if(lines < 2) {
            disp_free(disp);
            disp = NULL;
            break;
        }

        fseek(f, start_pos, SEEK_SET);

        disp_sample_table_init(dt, lines);

        table = dt->table_ref;

        for(j = 0; j < lines; j++) {
            float *dptr = table->heap + 3 * j;
            int read_status;
            do {
                read_status = fscanf(f, "%f %f %f\n", dptr, dptr+1, dptr+2);
            } while(read_status < 3 && read_status != EOF);

            if(convert_ev) {
                dptr[0] = 1239.8 / dptr[0];
            }

            if(provide_diel_k) {
                double e1 = dptr[1], e2 = dptr[2];
                double ne = sqrt(e1*e1 + e2*e2);
                dptr[1] = sqrt((ne + e1) / 2.0);
                dptr[2] = sqrt((ne - e1) / 2.0);
            }

            if(read_status == EOF) {
                break;
            }
        }

        break;
    }
    case DISP_CAUCHY:
        notify_error_msg(LOADING_FILE_ERROR, "cauchy MAT files");
        break;
#if 0
        cn = disp->disp.cauchy.n;
        ck = disp->disp.cauchy.k;
        fscanf(f, "%lf %lf %lf %*f %*f %*f\n", cn, cn+1, cn+2);
        cn[1] *= 1e3;
        cn[2] *= 1e6;
        ck[0] = ck[1] = ck[2] = 0.0;
        break;
#endif
    default:
        notify_error_msg(LOADING_FILE_ERROR, "corrupted material card");
        break;
    }

close_exit:
    fclose(f);
    str_free(row);
    return disp;
}

int
disp_sample_table_write(writer_t *w, const disp_t *_d)
{
    const struct disp_sample_table *d = &_d->disp.sample_table;
    writer_printf(w, "table \"%s\" %d", CSTR(_d->name), d->nb);
    writer_newline_enter(w);
    data_table_write(w, d->table_ref);
    writer_newline_exit(w);
    return 0;
}

int
disp_sample_table_read(lexer_t *l, disp_t *d_gen)
{
    int nb;
    if (lexer_integer(l, &nb)) return 1;
    struct data_table *tab = data_table_read(l);
    if (!tab) return 1;
    struct disp_sample_table *d = &d_gen->disp.sample_table;
    d->nb = nb;
    d->table_ref = tab;
    return 0;
}
