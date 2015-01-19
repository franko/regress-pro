#include "common.h"
#include "writer.h"

writer_t *writer_new()
{
    writer_t *w = emalloc(sizeof(writer_t));
    str_init(w->text, 1024);
    w->indent = 0;
    w->single_line = 0;
    return w;
};

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
        for (int i = 0; i < w->indent; i++) {
            str_append_c(w->text, "  ", 0);
        }
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
