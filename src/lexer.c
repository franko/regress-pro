#include <string.h>

#include "lexer.h"
#include "common.h"

lexer_t *
lexer_new(const char *s)
{
    lexer_t *l = emalloc(sizeof(lexer_t));
    l->text = s;
    str_init(l->buffer, 64);
    str_init(l->store, 64);
    lexer_next(l);
    return l;
}

void
lexer_free(lexer_t *l)
{
    str_free(l->buffer);
    str_free(l->store);
    free(l);
}

static int
read_string(const char *text, str_ptr buffer, const char **tail)
{
    const char *p;
    for (p = text; *p && *p != '"'; p++) {
        if (*p == '\\') { // Escape sequence.
            p ++;
            if (*p == '"') {
                str_append_char(buffer, '"');
            } else if (*p == 'n') {
                str_append_char(buffer, '\n');
            } else {
                return 1; // Invalid escape sequence.
            }
        } else {
            str_append_char(buffer, *p);
        }
    }
    if (*p == '"') {
        *tail = p + 1;
        return 0;
    }
    return 1;
}

static int
read_ident(const char *text, str_ptr buffer, const char **tail)
{
    const char *p;
    for (p = text; *p && ((*p >= 'a' && *p <= 'z') || *p == '-'); p++) {
        str_append_char(buffer, *p);
    }
    *tail = p;
    return (p == text ? 1 : 0);
}

void
lexer_next(lexer_t *l)
{
    while (*l->text == ' ' || *l->text == '\t' || *l->text == '\r' || *l->text == '\n') {
        l->text ++;
    }
    char c = l->text[0];
    if (c == 0) {
        l->current.tk = TK_EOF;
    } else if ((c >= '0' && c <= '9') || c == '.' || c == '-') {
        char *ftail, *ltail;
        double fval = strtod(l->text, &ftail);
        long lval = strtol(l->text, &ltail, 10);
        if (ltail == ftail) {
            l->current.tk = TK_INTEGER;
            l->current.value.integer = lval;
            l->text = ltail;
        } else {
            l->current.tk = TK_NUMBER;
            l->current.value.num = fval;
            l->text = ftail;
        }
    } else if (c == '"') {
        const char *tail;
        str_trunc(l->buffer, 0);
        if (read_string(l->text + 1, l->buffer, &tail)) {
            l->current.tk = TK_UNDEF;
        } else {
            l->current.tk = TK_STRING;
            l->current.value.str = CSTR(l->buffer);
            l->text = tail;
        }
    } else {
        const char *tail;
        str_trunc(l->buffer, 0);
        if (read_ident(l->text, l->buffer, &tail)) {
            l->current.tk = TK_UNDEF;
        } else {
            l->current.tk = TK_IDENT;
            l->current.value.str = CSTR(l->buffer);
            l->text = tail;
        }
    }
}

int
lexer_string_store(lexer_t *l, int tk_ident)
{
    const int req = (tk_ident ? TK_IDENT : TK_STRING);
    if (l->current.tk == req) {
        str_copy_c(l->store, l->current.value.str);
        lexer_next(l);
        return 0;
    }
    return 1;
}

int
lexer_check_ident(lexer_t *l, const char *id)
{
    if (l->current.tk == TK_IDENT && strcmp(l->current.value.str, id) == 0) {
        lexer_next(l);
        return 0;
    }
    return 1;
}

int
lexer_integer(lexer_t *l, int *value)
{
    if (l->current.tk == TK_INTEGER) {
        *value = l->current.value.integer;
        lexer_next(l);
        return 0;
    }
    return 1;
}

int
lexer_number(lexer_t *l, double *value)
{
    if (l->current.tk == TK_INTEGER) {
        *value = (double) (l->current.value.integer);
        lexer_next(l);
        return 0;
    } else if (l->current.tk == TK_NUMBER) {
        *value = l->current.value.num;
        lexer_next(l);
        return 0;
    }
    return 1;
}
