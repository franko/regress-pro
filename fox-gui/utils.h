#ifndef AGGPLOT_UTILS_H
#define AGGPLOT_UTILS_H

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

template <typename T> T min(T a, T b) {
    return (a < b) ? a : b;
};

template <typename T> T max(T a, T b) {
    return (a > b) ? a : b;
};

#endif
