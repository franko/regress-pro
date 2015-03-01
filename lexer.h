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
    int rem;
} lexer_t;

extern lexer_t * lexer_new(const char *s);
extern void      lexer_next(lexer_t *l);

__END_DECLS

#endif
