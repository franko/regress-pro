#ifndef ELLIPSOMETRY_DECLS_H
#define ELLIPSOMETRY_DECLS_H

#define THICKNESS_TO_NM(x) ((x) / 10.0)

#define DEGREE(d) ((d) * M_PI / 180.0)

typedef enum {
    POL_S = 0,
    POL_P = 1
} polar_t;

#endif