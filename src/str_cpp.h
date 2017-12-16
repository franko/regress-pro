#ifndef STR_CPP_H
#define STR_CPP_H

#include <cstring>

#include "str.h"
#include "str_priv.h"

static inline void null(_str& s) {
    s.heap = nullptr;
    s.size = 0;
    s.length = 0;
}

static inline void set(_str& dest, _str& src) {
    dest.heap = src.heap;
    dest.size = src.size;
    dest.length = src.length;
}

class str : public _str {
public:
    str() {
        str_init_raw(this, 64);
        heap[0] = 0;
        length = 0;
    }

    str(const str& s) {
        const size_t len = s.length;
        str_init_raw(this, len + 1);
        memcpy(heap, s.heap, len + 1);
        length = len;
    }

    str(str&& s) {
        set(*this, s);
        null(s);
    }

    str(const char *s) {
        const size_t len = strlen(s);
        str_init_raw(this, len + 1);
        memcpy(heap, s, len + 1);
        length = len;
    }

    ~str() {
        if (size > 0) {
            free(heap);
        }
        heap = 0;
    }

    str& operator=(const str& s) {
        str_copy(this, &s);
        return *this;
    }

    unsigned len() const { return this->length; }
    const char *text() const { return CSTR(this); }

    str& operator=(str&& s) {
        free(heap);
        set(*this, s);
        null(s);
        return *this;
    }

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


inline bool operator==(const str& a, const str& b) {
    return (a.len() == b.len() && strncmp(a.text(), b.text(), a.len() + 1) == 0);
}

inline bool operator!=(const str& a, const str& b) { return !(a == b); }

#endif
