#ifndef REGPRO_MATH_UTILS_H
#define REGPRO_MATH_UTILS_H

template <typename T> static inline T pow2(T x) { return x * x; }

template <typename T> static inline T pow4(T x) {
    const T s = x * x;
    return s * s;
}

#endif
