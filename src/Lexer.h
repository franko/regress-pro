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
    struct quoted_string : public str {
        explicit quoted_string() { }

        void set(const char *s) {
            str_copy_c(this, s);
        }
    };

    Lexer(const char *text);

    void next();

    void read(int& value);
    void read(double& value);
    void read(str& value);
    void read(quoted_string& value);

    void check_ident(const char *name);

    const char *lookup_ident() const {
        return (current.tk == TK_IDENT ? current.value.str : nullptr);
    }

    Token current;

private:
    const char *m_text;
    str m_buffer;
};

template <typename T>
Lexer& operator>>(Lexer& lexer, T& value) {
    lexer.read(value);
    lexer.next();
    return lexer;
}

Lexer& operator>>(Lexer& lexer, const char *s);

#endif
