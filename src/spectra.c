#include <assert.h>
#include <string.h>

#include "common.h"
#include "spectra.h"
#include "refl-utils.h"
#include "error-messages.h"
#include "data-table.h"
#include "str.h"
#include "fit-params.h"

static struct spectrum * load_ellips_spectrum(const char *filename, str_ptr *error_msg);

static double deg_to_radians(double x) { return x * M_PI / 180.0; }

struct spectrum *
load_ellips_spectrum(const char *filename, str_ptr *error_msg) {
    struct spectrum *s;
    struct data_table *data_table;
    int iseof, nr;
    str_t ln;
    FILE *f;

    f = fopen(filename, "r");

    if(f == NULL) {
        *error_msg = new_error_message(LOADING_FILE_ERROR, "File \"%s\" does not exists or cannot be opened", filename);
        return NULL;
    }

    str_init(ln, 64);

    s = emalloc(sizeof(struct spectrum));

    struct acquisition_parameters *acquisition = s->acquisition;
    acquisition_set_zero(acquisition);

    str_getline(ln, f);
    if(strstr(CSTR(ln), "SE ALPHA BETA")) {
        acquisition_set_default_rpe(acquisition);
    } else if(strstr(CSTR(ln), "SE PSI DELTA")) {
        acquisition_set_default_se(acquisition);
    } else {
        goto invalid_s;
    }

    double *aoi_ptr = acquisition_parameter_pointer(acquisition, PID_AOI);
    double *analyzer_ptr = acquisition_parameter_pointer(acquisition, PID_ANALYZER);

    for(;;) {
        iseof = str_getline(ln, f);

        if(iseof != 0) {
            break;
        }

        if (aoi_ptr) {
            nr = sscanf(CSTR(ln), "AOI %lf", aoi_ptr);
            if(nr == 1) {
                continue;
            }
        }

        if (analyzer_ptr) {
            nr = sscanf(CSTR(ln), "A %lf", analyzer_ptr);
            if(nr == 1) {
                continue;
            }
        }

        break;
    }

    do {
        float lambda, alpha, beta;

        nr = sscanf(CSTR(ln), "%*s %f %f %f\n", & lambda, & alpha, & beta);
        if(nr < 3) {
            goto invalid_s;
        }

        data_table = data_table_read_lines(f, "%*s %f %f %f\n", 1, 3);

        if(data_table == NULL) {
            goto invalid_s;
        }

        data_table_set(data_table, 0, 0, lambda);
        data_table_set(data_table, 0, 1, alpha);
        data_table_set(data_table, 0, 2, beta);
    } while(0);

    data_view_init(s->table, data_table);

    str_free(ln);
    fclose(f);

    return s;
invalid_s:
    *error_msg = new_error_message(LOADING_FILE_ERROR, "Format of spectra %s is incorrect", filename);
    free(s);
    str_free(ln);
    fclose(f);
    return NULL;
}

/* If defined to 1 will convert VASE Spectra to Alpha Beta form with
   a "fake" analyzer angle of 25 degree. */
#define VASE_CONVERT_TO_ALPHA_BETA 1

static struct spectrum *
load_vase_spectrum(const char *filename, str_ptr *error_msg)
{
    struct spectrum *s;
    struct data_table *data_table;

    FILE *f = fopen(filename, "r");
    if(f == NULL) {
        *error_msg = new_error_message(LOADING_FILE_ERROR, "File \"%s\" does not exists or cannot be opened", filename);
        return NULL;
    }

    str_t ln;
    str_init(ln, 64);
    str_getline(ln, f); /* Skip the first line. */
    str_getline(ln, f);
    if (!strstr(CSTR(ln), "nm")) {
        str_free(ln);
        *error_msg = new_error_message(LOADING_FILE_ERROR, "Format of spectra %s is incorrect", filename);
        return NULL;
    }

    s = emalloc(sizeof(struct spectrum));
    struct acquisition_parameters *acquisition = s->acquisition;
    acquisition_set_zero(acquisition);
    acquisition->type = (VASE_CONVERT_TO_ALPHA_BETA ? SYSTEM_SE_RPE : SYSTEM_SE);
    double tana;
    double *aoi_ptr;
    if (VASE_CONVERT_TO_ALPHA_BETA) {
        acquisition->parameters.rpe.aoi = 65.0;
        acquisition->parameters.rpe.analyzer = 25.0;
        aoi_ptr = &acquisition->parameters.rpe.aoi;
        tana = tanf(deg_to_radians(acquisition->parameters.rpe.analyzer));
    } else {
        acquisition->parameters.se.aoi = 65.0;
        aoi_ptr = &acquisition->parameters.se.aoi;
    }

    /* In order to know the AOI we lookup just the first line and
       reset the position just after. */
    long data_pos = ftell(f);
    int nread = fscanf(f, "E %*f %lf", aoi_ptr);
    if (nread == 0) goto invalid_vase;
    fseek(f, data_pos, SEEK_SET);

    /* The following function reads the integrality of the tabular data. */
    data_table = data_table_read_lines(f, "E %f %*f %f %f %*f %*f %*f %*f ", 0, 3);
    if(data_table == NULL) goto invalid_vase;

    int i;
    for (i = 0; i < data_table->rows; i++) {
        const float psi = data_table_get(data_table, i, 1);
        const float delta = data_table_get(data_table, i, 2);
        const float tpsi = tanf(deg_to_radians(psi));
        const float cosdelta = cosf(deg_to_radians(delta));
#if VASE_CONVERT_TO_ALPHA_BETA
        const float alpha = (tpsi*tpsi - tana*tana) / (tpsi*tpsi + tana*tana);
        const float beta = (2*tpsi*cosdelta*tana) / (tpsi*tpsi + tana*tana);
        data_table_set(data_table, i, 1, alpha);
        data_table_set(data_table, i, 2, beta);
#else
        data_table_set(data_table, i, 1, tpsi);
        data_table_set(data_table, i, 2, cosdelta);
#endif
    }

    data_view_init(s->table, data_table);

    str_free(ln);
    fclose(f);
    return s;

invalid_vase:
    *error_msg = new_error_message(LOADING_FILE_ERROR, "Format of spectra %s is incorrect", filename);
    free(s);
    str_free(ln);
    fclose(f);
    return NULL;

}

float
get_lambda_by_index(struct spectrum *s, int idx)
{
    assert(idx >= 0 && idx < s->table->rows);
    return data_view_get(s->table, idx, 0);
}

void
spectra_free(struct spectrum *s)
{
    data_view_dealloc(s->table);
    free(s);
}

struct spectrum *
load_gener_spectrum(const char *filename, str_ptr *error_msg)
{
    struct spectrum *spectr;
    str_t ln;
    FILE *f;

    f = fopen(filename, "r");

    if(f == NULL) {
        *error_msg = new_error_message(LOADING_FILE_ERROR, "File \"%s\" does not exists or cannot be opened", filename);
        return NULL;
    }

    str_init(ln, 64);
    str_getline(ln, f);

    fclose(f);

    if(strstr(CSTR(ln), "SE ALPHA BETA") || strstr(CSTR(ln), "SE PSI DELTA")) {
        spectr = load_ellips_spectrum(filename, error_msg);
    } else if(strstr(CSTR(ln), "VASE") || strstr(CSTR(ln), "M2000")) {
        spectr = load_vase_spectrum(filename, error_msg);
    } else if(strstr(CSTR(ln), "\"Wavelength (nm)\"") && strstr(CSTR(ln), "\"Reflectance\"")) {
        spectr = load_filmetrics_spectrum(filename, error_msg);
    } else {
        spectr = load_refl_data(filename, error_msg);
    }

    str_free(ln);

    return spectr;
}

void
spectr_cut_range(struct spectrum *s, float inf, float sup)
{
    int j, jmin = -1, npt = 0;

    data_view_reset(s->table);

    for(j = 0; j < spectra_points(s); j++) {
        float lam = get_lambda_by_index(s, j);

        if(lam < inf || lam > sup) {
            continue;
        }

        if(jmin < 0) {
            jmin = j;
        }

        npt++;
    }

    data_view_set_mask_offset(s->table, jmin, jmin + npt);
}

struct spectrum *
spectra_copy(const struct spectrum *src) {
    struct spectrum *copy = emalloc(sizeof(struct spectrum));
    copy->acquisition[0] = src->acquisition[0];
    data_view_copy(copy->table, src->table);
    return copy;
}

struct spectrum *
spectra_alloc(struct spectrum *s) {
    struct spectrum *synth = emalloc(sizeof(struct spectrum));
    int rows = spectra_points(s), cols = s->table->columns;
    struct data_table *table = data_table_new(rows, cols);
    synth->acquisition[0] = s->acquisition[0];
    data_view_init(synth->table, table);
    return synth;
}

void
spectra_resize(struct spectrum *s, int nr)
{
    struct data_view *dv = s->table;
    data_view_reset(dv);
    if(nr <= dv->table->rows) {
        data_view_set_mask_offset(dv, 0, nr);
    } else {
        data_view_dealloc(dv);
        dv->rows  = nr;
        dv->table = data_table_new(nr, dv->columns);
    }
}

float const *
spectra_get_values(struct spectrum const *s, int idx)
{
    return data_view_get_row(s->table, idx);
}

void
spectra_wavelength_range(const struct spectrum *s, double *wl_min, double *wl_max)
{
    int n = spectra_points(s);
    *wl_min = data_view_get(s->table, 0, 0);
    *wl_max = data_view_get(s->table, n-1, 0);
}
