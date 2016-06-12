#ifndef LEXER_H
#define LEXER_H

#include <memory>

#include "defs.h"
#include "str_cpp.h"

enum token_e {
    TK_UNDEF    = 0,
    TK_IDENT    = 1,
    TK_STRING,
    TK_EOF,
    TK_NUMBER   = 0x100,
    TK_INTEGER  = 0x101,
};

struct Token {
    enum token_e tk;
    union {
        double num;
        long integer;
        const char *str;
    } value;
};

class Lexer {
public:
    Lexer(const char *text);

    void next();
    int ident_to_store();
    int string_to_store();
    int integer(int *value);
    int number(double *value);

    int check_ident(const char *name);

    const char *lookup_ident() const {
        return (current.tk == TK_IDENT ? current.value.str : nullptr);
    }

    Token current;
    str store;
private:
    const char *m_text;
    str m_buffer;
};

#endif
