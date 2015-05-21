#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"

#ifndef DEBUG_MEM
void *
emalloc(int n)
{
    void *p;

    assert(n >= 0);

    p = malloc(n);

    if(p == NULL) {
        fprintf(stderr, "Couldn't allocate memory\n");
        fflush(stderr);
        exit(1);
    }

    return p;
}

void *
erealloc(void *p, int n)
{
    void *newp;

    assert(n >= 0);

    newp = realloc(p, n);

    if(newp == NULL) {
        fprintf(stderr, "Couldn't allocate memory\n");
        fflush(stderr);
        exit(1);
    }
    return newp;
}
#endif

void
generic_array_check_alloc(struct generic_array *s, int idx,
                          size_t data_size)
{
    void *oldptr;

    assert(idx >= 0);

    if((size_t)idx < s->alloc) {
        return;
    }

    oldptr = s->heap;
    do {
        s->alloc *= 2;
    } while((size_t)idx >= s->alloc);

    s->heap = emalloc(s->alloc * data_size);
    memcpy(s->heap, oldptr, s->number * data_size);
    free(oldptr);
}

void *
generic_array_new(size_t data_size, size_t nb_init)
{
    struct generic_array *r;

    r = emalloc(sizeof(struct generic_array));

    r->number = 0;
    r->alloc = nb_init;
    r->heap = emalloc(r->alloc * data_size);

    return (void *) r;
}

void
generic_array_free(struct generic_array *r)
{
    free(r->heap);
    free(r);
}
