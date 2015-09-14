#ifndef STR_CPP_H
#define STR_CPP_H

#include "str.h"

class str : public _str {
public:
    str() {
        str_init(this, 64);
    }

    str(const str& s) {
        str_init_from_str(this, &s);
    }

    str(const char *t) {
        str_init_from_c(this, t);
    }

    ~str() {
        str_free(this);
    }

    str& operator=(const str& s) {
        str_copy(this, &s);
        return *this;
    }

    unsigned len() const { return this->length; }
    const char *text() const { return CSTR(this); }

#if 0
    // Preparing C++11 transition
    str& operator=(str&& s) {
        STR_MEMCPY(this, &s);
        STR_NULL(&s);
        return *this;
    }
#endif

    str& operator=(const char *s) {
        str_copy_c(this, s);
        return *this;
    }

    static str format(const char *fmt, ...) {
        str new_string;
        va_list ap;
        va_start(ap, fmt);
        str_vprintf(&new_string, fmt, 0, ap);
        va_end(ap);
        return new_string;
    }

    str& operator+=(const str& s) {
        str_append(this, &s, 0);
        return *this;
    }

    str& operator+=(const char *s) {
        str_append_c(this, s, 0);
        return *this;
    }

    friend str operator+(str lhs, const str& rhs) {
        lhs += rhs;
        return lhs;
    }
};

#endif
