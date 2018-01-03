#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include "defs.h"

#ifdef DEBUG_MEM
#define emalloc(n) malloc((size_t) (n))
#define erealloc(x,n) realloc(x,(size_t) (n))
#else
extern void *   emalloc(int n);
extern void *   erealloc(void *p, int n);
#endif

#ifdef WIN32
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif

#endif
