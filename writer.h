#ifndef WRITER_H
#define WRITER_H

#include "str.h"

struct _writer {
    str_t text;
    int indent;
    int single_line;
};

typedef struct _writer writer_t;

extern writer_t *writer_new();
extern void      writer_free(writer_t *w);
extern void      writer_printf(writer_t *w, const char *fmt, ...);
extern void      writer_newline(writer_t *w);
extern void      writer_newline_enter(writer_t *w);
extern void      writer_newline_exit(writer_t *w);

#endif
