#include "common.h"
#include "writer.h"

writer_t *writer_new()
{
    writer_t *w = emalloc(sizeof(writer_t));
    str_init(w->text, 1024);
    w->indent = 0;
    w->single_line = 0;
    w->new_line = 1;
    return w;
};

static void
begin_write(writer_t *w)
{
    if (w->new_line) {
        int i;
        for (i = 0; i < w->indent; i++) {
            str_append_c(w->text, "  ", 0);
        }
        w->new_line = 0;
    }
}

void
writer_free(writer_t *w)
{
    str_free(w->text);
    free(w);
}

void
writer_printf(writer_t *w, const char *fmt, ...)
{
    va_list ap;
    begin_write(w);
    va_start(ap, fmt);
    str_vprintf(w->text, fmt, 1, ap);
    va_end(ap);
}

void
writer_newline(writer_t *w)
{
    if (w->single_line) {
        str_append_c(w->text, " ", 0);
    } else {
        str_append_c(w->text, "\n", 0);
        w->new_line = 1;
    }
}

void
writer_newline_enter(writer_t *w)
{
    w->indent ++;
    writer_newline(w);
}

void
writer_newline_exit(writer_t *w)
{
    w->indent --;
    writer_newline(w);
}

void
writer_indent(writer_t *w, int n)
{
    w->indent += n;
}
