#ifndef CMPL_VECTOR_H
#define CMPL_VECTOR_H

#include <cmath>
#include <complex>
#include "common.h"
#include "stack_array.h"
#include "pod_vector.h"

typedef std::complex<double> cmpl;

template<class T> std::complex<T> operator*(const int val, const std::complex<T>& rhs) {
    return std::complex<T>(val * rhs.real(), val * rhs.imag());
}

enum complex_part_e { REAL_PART, IMAGINARY_PART };

static inline double complex_part(cmpl z, complex_part_e part) {
    return (part == REAL_PART ? std::real(z) : std::imag(z));
}

typedef StackArray<cmpl, 8>  cmpl_array8;
typedef StackArray<cmpl, 16> cmpl_array16;
typedef StackArray<cmpl, 24> cmpl_array24;
typedef StackArrayBase<cmpl> cmpl_array;

typedef StackArray<double, 4>  double_array4;
typedef StackArray<double, 8>  double_array8;
typedef StackArray<double, 16> double_array16;
typedef StackArray<double, 24> double_array24;
typedef StackArrayBase<double> double_array;

typedef pod::array<cmpl> cmpl_vector;

#endif
