#ifndef WRITER_H
#define WRITER_H

#include "defs.h"
#include "str.h"

__BEGIN_DECLS

struct _writer {
    str_t text;
    int indent;
    int single_line;
    int new_line;
};

typedef struct _writer writer_t;

extern writer_t *writer_new();
extern void      writer_free(writer_t *w);
extern void      writer_printf(writer_t *w, const char *fmt, ...);
extern void      writer_newline(writer_t *w);
extern void      writer_indent(writer_t *w, int n);
extern void      writer_newline_enter(writer_t *w);
extern void      writer_newline_exit(writer_t *w);

__END_DECLS

#endif
