#ifndef VECTOR_PRINT_H
#define VECTOR_PRINT_H

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

#include "str.h"

extern void print_vector(str_t s, const char *fmt, const gsl_vector *v);
extern void print_matrix(str_t s, const char *fmt, const gsl_matrix *m);

#endif
