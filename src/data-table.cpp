#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "data-table.h"
#include "generic_pod_vector.h"

struct data_table empty_data_table[1] = {{0, 0, -1, {0.0}}};

struct data_table *
data_table_read_lines(FILE *f, const char *fmt, int row_start, int columns) {
    pod_vector_base<float> table(256);
    struct data_table *r = nullptr;
    int rows, nread, j;

    for(j = 0, rows = 0; /* */; rows++, j += columns) {
        float val[3];
        if(columns == 2) {
            nread = fscanf(f, fmt, &val[0], &val[1]);
        } else if(columns == 3) {
            nread = fscanf(f, fmt, &val[0], &val[1], &val[2]);
        } else {
            assert(0);
        }

        table.add_array(val, columns);

        if(nread < columns) {
            if(feof(f) && rows >= 2) {
                r = data_table_new(rows + row_start, columns);
                const float *table_data = table.data();
                std::copy(table_data, table_data + rows * columns, r->heap + columns * row_start);
            }
            break;
        }
    }
    return r;
}

struct data_table *
data_table_new(int rows, int columns) {
    struct data_table *r;
    int excess_size = rows * columns - 1;

    assert(rows > 0 && columns > 0);

    r = (struct data_table *) emalloc(sizeof(struct data_table) + excess_size * sizeof(float));

    r->rows      = rows;
    r->columns   = columns;
    r->ref_count = 1;

    return r;
}

void
data_table_unref(struct data_table *table)
{
    /* for statically allocated data_table ref_count < 0 */
    if(table->ref_count < 0) {
        return;
    }

    table->ref_count --;

    assert(table->ref_count >= 0);

    if(table->ref_count == 0) {
        free(table);
    }
}

int
data_table_write(writer_t *w, const struct data_table *dt)
{
    writer_printf(w, "%d %d", dt->rows, dt->columns);
    writer_newline(w);
    int i, j;
    for (i = 0; i < dt->rows; i++) {
        if (i > 0) {
            writer_newline(w);
        }
        for (j = 0; j < dt->columns; j++) {
            if (j > 0) {
                writer_printf(w, " ");
            }
            writer_printf(w, "%g", data_table_get(dt, i, j));
        }
    }
    return 0;
}

struct data_table *
data_table_read(lexer_t *l)
{
    int rows, columns;
    if (lexer_integer(l, &rows)) return nullptr;
    if (lexer_integer(l, &columns)) return nullptr;
    int i, j;
    struct data_table *dt = data_table_new(rows, columns);
    float *p = dt->heap;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < columns; j++, p++) {
            double x;
            if (lexer_number(l, &x)) goto table_exit;
            *p = x;
        }
    }
    return dt;
table_exit:
    data_table_unref(dt);
    return nullptr;
}
