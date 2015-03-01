#include <string.h>

#include "lexer.h"
#include "common.h"

lexer_t *
lexer_new(const char *s)
{
    lexer_t *l = emalloc(sizeof(lexer_t));
    l->text = s;
    l->rem = strlen(s);
    l->current.tk = TK_UNDEF;
    str_init(l->buffer, 64);
    return l;
}

static void
read_string(const char *text, str_ptr buffer, const char **tail)
{
    char c[2] = {0, 0};
    const char *p;
    for (p = text; *p && *p != '"'; p++) {
        c[0] = *p;
        str_append_c(buffer, c, 0);
    }
    if (*p == '"') {
        *tail = p + 1;
    } else {
        *tail = text;
    }
}

static void
read_ident(const char *text, str_ptr buffer, const char **tail)
{
    char c[2] = {0, 0};
    const char *p;
    for (p = text; *p && ((*p >= 'a' && *p <= 'z') || *p == '-'); p++) {
        c[0] = *p;
        str_append_c(buffer, c, 0);
    }
    *tail = p;
}

void
lexer_next(lexer_t *l)
{
    while (*l->text == ' ' || *l->text == '\n') {
        l->text ++;
        l->rem --;
    }
    char c = l->text[0];
    if (c == 0) {
        l->current.tk = TK_EOF;
    } else if ((c >= '0' && c <= '9') || c == '.') {
        char *ftail, *ltail;
        double fval = strtod(l->text, &ftail);
        long lval = strtol(l->text, &ltail, 10);
        if (ltail == ftail) {
            l->current.tk = TK_INTEGER;
            l->current.value.integer = lval;
            l->rem -= (ltail - l->text);
            l->text = ltail;
        } else {
            l->current.tk = TK_NUMBER;
            l->current.value.num = fval;
            l->rem -= (ftail - l->text);
            l->text = ftail;
        }
    } else if (c == '"') {
        const char *tail;
        str_trunc(l->buffer, 0);
        read_string(l->text + 1, l->buffer, &tail);
        l->current.tk = TK_STRING;
        l->current.value.str = CSTR(l->buffer);
        l->rem -= (tail - l->text);
        l->text = tail;
    } else {
        const char *tail;
        str_trunc(l->buffer, 0);
        read_ident(l->text, l->buffer, &tail);
        if (tail == l->text) {
            l->current.tk = TK_UNDEF;
        } else {
            l->current.tk = TK_STRING;
            l->current.value.str = CSTR(l->buffer);
            l->rem -= (tail - l->text);
            l->text = tail;
        }
    }
}
