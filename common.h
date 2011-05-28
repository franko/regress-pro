#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include "defs.h"

__BEGIN_DECLS

#define DEGREE(d) ((d) * M_PI / 180.0)
#define SQR(x) ((x) * (x))

#ifdef DEBUG_MEM
#define emalloc(n) malloc((size_t) (n))
#define erealloc(x,n) realloc(x,(size_t) (n))
#else
extern void *   emalloc                 (int n);
extern void *   erealloc                (void *p, int n);
#endif

#ifdef WIN32
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif

struct generic_array {
  size_t number;
  size_t alloc;
  void *heap;
};

extern void   generic_array_check_alloc   (struct generic_array *s, int index,
					   size_t data_size);
extern void * generic_array_new           (size_t data_size);
extern void   generic_array_free          (struct generic_array *r);

#define ARRAY_CHECK_ALLOC(s,dtype,idx) generic_array_check_alloc((struct generic_array *) (s),idx,sizeof(dtype))
#define ARRAY_NEW(dtype) generic_array_new(sizeof(dtype))
#define ARRAY_FREE(s) generic_array_free(s)

__END_DECLS

#endif
