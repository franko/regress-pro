#include <stdio.h>
#include <string.h>

#include "refl-utils.h"
#include "error-messages.h"
#include "data-table.h"
#include "number-parse.h"

#define NORMALIZE(c) ((float) c / 32020.0)

static double
get_lambda(double w[], int k)
{
    return w[0] + k * w[1] + k*k * w[2];
}

struct data_table *
read_nova_spectrum(FILE *f, str_ptr ln) {
    struct data_table *table;
    unsigned int c, c_save;
    int na, j, lc0 = 0, lcount;
    long dpos;
    int starting_zeroes = 1;
    int ezcount = 0; /* Number of zeroes at the end of file. */
    double w[5];

    for(;;) {
        char tc;
        if(str_getline(ln, f) < 0) {
            break;
        }
        na = sscanf(CSTR(ln), "%u %c", &c, &tc);

        if(na == 1) {
            if(c > 0 && starting_zeroes) {
                c_save = c;
                dpos = ftell(f);
                starting_zeroes = 0;
                lcount = 1;
                continue;
            } else if (c > 0 && ezcount > 0) {
                lcount += ezcount;
                ezcount = 0;
            }

            if(c == 0) {
                if(!starting_zeroes) {
                    ezcount ++;
                } else {
                    lc0++;
                }
            }

            if(!starting_zeroes && ezcount == 0) {
                lcount++;
            }
            continue;
        } else if(na == 2 && lcount > 0) {
            const char *p = CSTR(ln);
            if(sscanf(p, "%lf %lf %lf %lf %lf \n", w, w+1, w+2, w+3, w+4) == 5) {
                break;
            }
        }

        return NULL;
    }

    if(fseek(f, dpos, SEEK_SET) < 0) {
        return NULL;
    }

    table = data_table_new(lcount, 2);
    data_table_set(table, 0, 0, get_lambda(w, lc0+1));
    data_table_set(table, 0, 1, NORMALIZE(c_save));

    for(j = 1; j < lcount; j++) {
        fscanf(f, " %u", &c);

        data_table_set(table, j, 0, get_lambda(w, lc0+j+1));
        data_table_set(table, j, 1, NORMALIZE(c));
    }

    return table;
}

struct spectrum *
load_refl_data(const char *filename, str_ptr *error_msg) {
    struct spectrum *s;
    struct data_table *table;
    str_t ln;
    FILE *f;

    f = fopen(filename, "r");

    if(f == NULL) {
        *error_msg = new_error_message(LOADING_FILE_ERROR, "File \"%s\" does not exists or cannot be opened", filename);
        return NULL;
    }

    str_init(ln, 64);

    s = emalloc(sizeof(struct spectrum));
    s->config.system = SYSTEM_REFLECTOMETER;

    str_getline(ln, f);

    if(strstr(CSTR(ln), ";Experimental Spectrum") || \
            strstr(CSTR(ln), ";Theoretical Spectrum")) {
        table = read_nova_spectrum(f, ln);
    } else {
        str_getline(ln, f);
        table = data_table_read_lines(f, "%*s %f %*f %f %*f\n", 0, 2);
    }

    if(table == NULL) {
        goto invalid_s;
    }

    data_view_init(s->table, table);

    str_free(ln);
    fclose(f);

    return s;
invalid_s:
    *error_msg = new_error_message(LOADING_FILE_ERROR, "Format of spectra \"%s\" is incorrect", filename);
    free(s);
    str_free(ln);
    fclose(f);
    return NULL;
}

static int filmetrics_parse_line(const char *s, unsigned int flags, float x[])
{
    int n;
    const char list_sep = (flags & PARSE_FLOAT_FRENCH_LOCALE ? ';' : ',');
    if (parse_float(s, flags, &x[0], &n)) return 1;
    s = skip_spaces(s + n);
    if (*s != list_sep && *s != '\t') return 1;
    s = skip_spaces(s + 1);
    if (parse_float(s, flags, &x[1], &n)) return 1;
    s = skip_spaces(s + n);
    while (*s == '\r' || *s == '\n') {
        s ++;
    }
    return (*s == '\0' ? 0 : 1);
}

static int filmetrics_test_line_format(const char *s)
{
    float x[2];
    unsigned int test_flags = PARSE_FLOAT_SKIP_SPACES;
    if (filmetrics_parse_line(s, test_flags, x) == 0) {
        return test_flags;
    }
    test_flags |= PARSE_FLOAT_FRENCH_LOCALE;
    if (filmetrics_parse_line(s, test_flags, x) == 0) {
        return test_flags;
    }
    return -1;
}

static struct data_table *
filmetrics_read_data_table(FILE *f, unsigned parse_flags, str_ptr ln) {
    const int columns = 2;
    struct generic_array *data = ARRAY_NEW(float);
    struct data_table *r = NULL;
    int row, index = 0;

    for(row = 0; /* */; row++, index += columns) {
        ARRAY_CHECK_ALLOC(data, float, index + columns - 1);
        data->number += columns;
        str_getline(ln, f);
        if (filmetrics_parse_line(CSTR(ln), parse_flags, ARRAY_GET_PTR(data, float, index))) {
            if(feof(f) && row >= 2) {
                r = data_table_new(row, columns);
                memcpy(r->heap, data->heap, row * columns * sizeof(float));
            }
            break;
        }
    }
    ARRAY_FREE(data);
    return r;
}

struct spectrum *
load_filmetrics_spectrum(const char *filename, str_ptr *error_msg) {
    struct spectrum *s;
    str_t ln;
    FILE *f;

    f = fopen(filename, "r");

    if(f == NULL) {
        *error_msg = new_error_message(LOADING_FILE_ERROR, "File \"%s\" does not exists or cannot be opened", filename);
        return NULL;
    }

    s = emalloc(sizeof(struct spectrum));
    s->config.system = SYSTEM_REFLECTOMETER;

    str_init(ln, 64);
    str_getline(ln, f);

    long data_pos = ftell(f);
    str_getline(ln, f);

    int line_flags = filmetrics_test_line_format(CSTR(ln));
    if (line_flags < 0) {
        *error_msg = new_error_message(LOADING_FILE_ERROR, "Unknown Filmetrics file format");
        goto filmetrics_fail;
    }

    if (fseek(f, data_pos, SEEK_SET) != 0) {
        *error_msg = new_error_message(LOADING_FILE_ERROR, "cannot seek file");
        goto filmetrics_fail;
    }

    struct data_table *table = filmetrics_read_data_table(f, line_flags, ln);
    if (!table) goto filmetrics_fail;

    data_view_init(s->table, table);

    str_free(ln);
    fclose(f);
    return s;

filmetrics_fail:
    *error_msg = new_error_message(LOADING_FILE_ERROR, "Format of spectra \"%s\" is incorrect", filename);
    free(s);
    str_free(ln);
    fclose(f);
    return NULL;
}
