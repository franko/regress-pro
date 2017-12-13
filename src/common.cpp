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

    if(p == nullptr) {
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

    if(newp == nullptr) {
        fprintf(stderr, "Couldn't allocate memory\n");
        fflush(stderr);
        exit(1);
    }
    return newp;
}
#endif
