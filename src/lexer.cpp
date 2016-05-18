#include <string.h>

#include "lexer.h"

Lexer::Lexer(const char *s) : m_text(s) {
    next();
}

static void
read_string(const char *text, str& buffer, const char **tail)
{
    const char *p;
    for (p = text; *p && *p != '"'; p++) {
        buffer += *p;
    }
    if (*p == '"') {
        *tail = p + 1;
    } else {
        *tail = text;
    }
}

static void
read_ident(const char *text, str& buffer, const char **tail)
{
    const char *p;
    for (p = text; *p && ((*p >= 'a' && *p <= 'z') || *p == '-'); p++) {
        buffer += *p;
    }
    *tail = p;
}

void Lexer::next() {
    while (*m_text == ' ' || *m_text == '\t' || *m_text == '\r' || *m_text == '\n') {
        m_text ++;
    }
    char c = m_text[0];
    if (c == 0) {
        current.tk = TK_EOF;
    } else if ((c >= '0' && c <= '9') || c == '.' || c == '-') {
        char *ftail, *ltail;
        double fval = strtod(m_text, &ftail);
        long lval = strtol(m_text, &ltail, 10);
        if (ltail == ftail) {
            current.tk = TK_INTEGER;
            current.value.integer = lval;
            m_text = ltail;
        } else {
            current.tk = TK_NUMBER;
            current.value.num = fval;
            m_text = ftail;
        }
    } else if (c == '"') {
        const char *tail;
        m_buffer.clear();
        read_string(m_text + 1, m_buffer, &tail);
        current.tk = TK_STRING;
        current.value.str = m_buffer.text();
        m_text = tail;
    } else {
        const char *tail;
        m_buffer.clear();
        read_ident(m_text, m_buffer, &tail);
        if (tail == m_text) {
            current.tk = TK_UNDEF;
        } else {
            current.tk = TK_IDENT;
            current.value.str = m_buffer.text();
            m_text = tail;
        }
    }
}

int Lexer::ident_to_store() {
    if (current.tk == TK_IDENT) {
        store = current.value.str;
        next();
        return 0;
    }
    return 1;
}

int Lexer::string_to_store() {
    if (current.tk == TK_STRING) {
        store = current.value.str;
        next();
        return 0;
    }
    return 1;
}

int Lexer::check_ident(const char *id) {
    if (current.tk == TK_IDENT && strcmp(current.value.str, id) == 0) {
        next();
        return 0;
    }
    return 1;
}

int Lexer::integer(int *value) {
    if (current.tk == TK_INTEGER) {
        *value = current.value.integer;
        next();
        return 0;
    }
    return 1;
}

int Lexer::number(double *value)
{
    if (current.tk == TK_INTEGER) {
        *value = (double) (current.value.integer);
        next();
        return 0;
    } else if (current.tk == TK_NUMBER) {
        *value = current.value.num;
        next();
        return 0;
    }
    return 1;
}
