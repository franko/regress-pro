#ifndef CMPL_VECTOR_H
#define CMPL_VECTOR_H

#include <cmath>
#include <complex>
#include "common.h"

typedef std::complex<double> cmpl;

template<class T> std::complex<T> operator*(const int val, const std::complex<T>& rhs) {
    return std::complex<T>(val * rhs.real(), val * rhs.imag());
}

enum complex_part_e { REAL_PART, IMAGINARY_PART };

static inline double complex_part(cmpl z, complex_part_e part) {
    return (part == REAL_PART ? std::real(z) : std::imag(z));
}

struct cmpl_vector_struct {
    int size; /* number of cmpl elements */
    cmpl * data;
    int owner;
};

typedef struct cmpl_vector_struct cmpl_vector;

cmpl_vector *   cmpl_vector_alloc(int nb);
void            cmpl_vector_free(cmpl_vector *v);
void            cmpl_vector_set(cmpl_vector *v, int i, cmpl val);
cmpl            cmpl_vector_get(cmpl_vector *v, int i);

#endif
