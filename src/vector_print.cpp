#include "vector_print.h"

void
print_vector(str_t s, const char *fmt, const gsl_vector *v)
{
    size_t j;
    for(j = 0; j < v->size; j++) {
        if(j > 0) {
            str_append_c(s, " ", 0);
        }
        str_printf_add(s, fmt, gsl_vector_get(v, j));
    }
    str_append_c(s, "\n", 0);
}

void
print_matrix(str_t s, const char *fmt, const gsl_matrix *m)
{
    int field_size = 0;
    size_t i, j;
    size_t n1 = m->size1, n2 = m->size2;
    str_t number;

    str_init(number, 64);

    for(i = 0; i < n1; i++) {
        for(j = 0; j < n2; j++) {
            str_printf(number, fmt, gsl_matrix_get(m, i, j));
            if(STR_LENGTH(number) > field_size) {
                field_size = STR_LENGTH(number);
            }
        }
    }

    for(i = 0; i < n1; i++) {
        str_append_c(s, "|", 0);
        for(j = 0; j < n2; j++) {
            if(j > 0) {
                str_append_c(s, " ", 0);
            }

            str_printf(number, fmt, gsl_matrix_get(m, i, j));
            str_pad(number, field_size, ' ');

            str_append(s, number, 0);
        }
        str_append_c(s, "|\n", 0);
    }

    str_free(number);
}
