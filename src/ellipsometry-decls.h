#ifndef ELLIPSOMETRY_DECLS_H
#define ELLIPSOMETRY_DECLS_H

#define THICKNESS_TO_NM(x) ((x) * 0.1)

#define DEGREE(d) ((d) * 0.01745329251994329576923690768489)

typedef enum {
    POL_S = 0,
    POL_P = 1
} polar_t;

#endif
