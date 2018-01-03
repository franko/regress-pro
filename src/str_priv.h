#ifndef REGPRO_STR_PRIVATE_H
#define REGPRO_STR_PRIVATE_H

#include "str.h"
#include "common.h"

/* Increase the required allocation size. */
#define ALLOC_SIZE_INCREASE(x) (((x) * 3) / 2)

#define SIZE_ALIGN_SHIFT 3
#define SIZE_ALIGN (1 << SIZE_ALIGN_SHIFT)
#define SIZE_ALIGN_MASK (SIZE_ALIGN - 1)

/* Roundup the size of allocated memory. */
#define STR_SIZE_ROUND(s) (((s) + SIZE_ALIGN_MASK) & ~SIZE_ALIGN_MASK)

/* Minimum size of allocation. */
#define STR_MIN_SIZE 8

static inline void str_init_raw(str_ptr s, size_t len) {
    const size_t alloc_size = STR_SIZE_ROUND(len+1);
    s->size = (alloc_size >= STR_MIN_SIZE ? alloc_size : STR_MIN_SIZE);
    s->heap = (char *) emalloc(s->size);
}

#endif
