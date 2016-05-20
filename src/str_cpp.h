#ifndef STR_CPP_H
#define STR_CPP_H

#include <cstring>

#include "str.h"

class str : public _str {

public:
    /* This class provides a save pointer to the end of the string to
       efficiently and safely append text.
       It provides a sprintf method to append formatted text. */
    class pointer {

        int capacity() const { return m_containing_string->size - m_containing_string->length; }

        char *ptr() { return m_containing_string->heap + m_containing_string->length; }

    public:
        pointer(str *s, int capacity): m_containing_string(s) {
            str_size_check(s, s->length + capacity - 1);
        }

        void copy(const char *s) {
            int len = strlen(s);
            str_size_check(m_containing_string, m_containing_string->length + len);
            std::memcpy(ptr(), s, len + 1);
            m_containing_string->length += len;
        }

        void sprintf(const char *fmt, ...) {
            va_list ap;
            va_start(ap, fmt);
            int len = vsnprintf(ptr(), capacity(), fmt, ap);
            va_end(ap);
            if (len >= capacity()) {
                str_size_check(m_containing_string, m_containing_string->length + len);
                va_start(ap, fmt);
                vsnprintf(ptr(), len + 1, fmt, ap);
                va_end(ap);
            }
            m_containing_string->length += len;
        }

    private:
        str *m_containing_string;
    };

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

    pointer append_pointer(int extra_size = 0) {
        return pointer(this, extra_size);
    }

    void clear() {
        str_trunc(this, 0);
    }

    str& operator=(str&& s) {
        STR_MEMCPY(this, &s);
        STR_NULL(&s);
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

    str& operator+=(const char ch) {
        char s[2] = {ch, 0};
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
