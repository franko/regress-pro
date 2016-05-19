#ifndef WRITER_H
#define WRITER_H

#include "defs.h"
#include "str_cpp.h"

class Writer {
public:
    Writer();

    template <typename T>
    void append(const T& s) {
        if (m_pending_space) {
            m_text += " ";
        }
        m_text += s;
        m_pending_space = true;
    }
    void printf(const char *fmt, ...);
    void newline();
    void newline_enter();
    void newline_exit();
    int save_tofile(const char *filename);

    void indent(int n) { m_indent += n; }

private:
    void begin_write();

    str m_text;
    int m_indent;
    bool m_new_line;
    bool m_pending_space;
};

inline str to_string(double x)      { return str::format("%g", x); }
inline str to_string(int x)         { return str::format("%d", x); }
inline str to_string(const char *s) { return str(s); }

template <typename T>
Writer& operator<<(Writer& w, const T& value) {
    w.append(to_string(value));
    return w;
}

#endif

