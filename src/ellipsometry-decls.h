#ifndef ELLIPSOMETRY_DECLS_H
#define ELLIPSOMETRY_DECLS_H

#define THICKNESS_TO_NM(x) ((x) / 10.0)

typedef enum {
    POL_S = 0,
    POL_START = POL_S,
    POL_P = 1,
    POL_END = POL_P + 1,
} polar_t;

#endif