#ifndef WRITER_H
#define WRITER_H

#include <cstring>

#include <EASTL/vector.h>

#include "defs.h"
#include "str_cpp.h"

class Writer {
public:
    struct quoted_string {
        const str& text;
    };

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

    void append(const quoted_string& quoted) {
        append_quoted(quoted.text);
    }

    void append_quoted(const str& s) {
        const int len = s.len();
        str::pointer p = m_text.append_pointer(len + 2);
        p.copy('"');
        const char *s_text = s.text();
        for (int i = 0; i < len; i++) {
            const char c = s_text[i];
            if (c == '"') {
                p.copy('\\');
                p.copy('"');
            } else {
                p.copy(c);
            }
        }
        p.copy('"');
    }

    void append(const str& s) {
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

template <typename T>
Writer& operator<<(Writer& w, const eastl::vector<T>& vec) {
    w.append(int(vec.size()));
    w.indent(+1);
    for (auto element : vec) {
        w.newline();
        w << element;
    }
    w.indent(-1);
    w.newline();
    return w;
}

#endif
