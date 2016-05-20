#ifndef WRITER_H
#define WRITER_H

#include <cstring>

#include "defs.h"
#include "str_cpp.h"

class Writer {
public:
    Writer();

    void printf(const char *fmt, ...);

    void newline_enter();
    void newline_exit();
    int save_tofile(const char *filename);

    void indent(int n) { m_indent += n; }

    bool pending_space() const { return m_pending_space; }
    void pending_space(bool value) { m_pending_space = value; }

    void newline() {
        m_text += "\n";
        m_new_line = true;
        m_pending_space = false;
    }

    void append(double x) {
        str::pointer p = m_text.append_pointer(16);
        p.sprintf("%g", x);
    }

    void append(int x) {
        str::pointer p = m_text.append_pointer(16);
        p.sprintf("%d", x);
    }

    void append(const char *s) {
        str::pointer p = m_text.append_pointer();
        p.copy(s);
    }

private:
    void begin_write();

    str m_text;
    int m_indent;
    bool m_new_line;
    bool m_pending_space;
};

template <typename T>
Writer& operator<<(Writer& w, const T& value) {
    if (w.pending_space()) {
        w.append(" ");
    }
    w.append(value);
    w.pending_space(true);
    return w;
}

#endif
