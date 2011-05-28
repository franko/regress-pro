#ifndef CMPL_VECTOR_H
#define CMPL_VECTOR_H

#include <math.h>

#ifdef __cplusplus
#define complex _Complex
#else
#include <complex.h>
#endif /* C++ */

#include "common.h"

__BEGIN_DECLS

#define CMPL_VECTOR_TERM(v,i) (v)->data[i]
#define CSQABS(z) (creal(z)*creal(z) + cimag(z)*cimag(z))

typedef double complex cmpl;

struct cmpl_vector_struct {
  int size; /* number of cmpl elements */
  cmpl * data;
  int owner;
};

typedef struct cmpl_vector_struct cmpl_vector;

cmpl_vector *   cmpl_vector_alloc       (int nb);
void            cmpl_vector_free        (cmpl_vector *v);
void            cmpl_vector_set         (cmpl_vector *v, int i, cmpl val);
cmpl            cmpl_vector_get         (cmpl_vector *v, int i);

__END_DECLS

#endif
