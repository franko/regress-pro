#ifndef WRITER_H
#define WRITER_H

#include "defs.h"
#include "str.h"

class Writer {
public:
    Writer();
    ~Writer();

    void printf(const char *fmt, ...);
    void newline();
    void newline_enter();
    void newline_exit();
    int save_tofile(const char *filename);

    void indent(int n) { m_indent += n; }

private:
    void begin_write();

    str_t m_text;
    int m_indent;
    bool m_new_line;
};

#endif
