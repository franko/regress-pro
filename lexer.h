#ifndef LEXER_H
#define LEXER_H

#include "defs.h"
#include "str.h"

__BEGIN_DECLS

enum token_e {
    TK_UNDEF    = 0,
    TK_IDENT    = 1,
    TK_STRING,
    TK_EOF,
    TK_NUMBER   = 0x100,
    TK_INTEGER  = 0x101,
};

typedef struct {
    enum token_e tk;
    union {
        double num;
        long integer;
        const char *str;
    } value;
} token_t;

typedef struct {
    const char *text;
    token_t current;
    str_t buffer;
    str_t store;
} lexer_t;

extern lexer_t * lexer_new(const char *s);
extern void      lexer_free(lexer_t *l);
extern void      lexer_next(lexer_t *l);
extern int       lexer_string_store(lexer_t *l, int tk_ident);
extern int       lexer_integer(lexer_t *l, int *value);
extern int       lexer_number(lexer_t *l, double *value);

#define lexer_string(a) lexer_string_store(a, 0)
#define lexer_ident(a) lexer_string_store(a, 1)

__END_DECLS

#endif
