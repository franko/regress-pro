#ifndef REGPRO_MATH_CONSTANTS_H
#define REGPRO_MATH_CONSTANTS_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline double deg_to_radians(double x) { return x * M_PI / 180.0; }

#endif
