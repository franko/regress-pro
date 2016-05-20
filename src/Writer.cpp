#include "common.h"
#include "Writer.h"

Writer::Writer() : m_indent(0), m_new_line(true), m_pending_space(false) { };

void Writer::begin_write() {
    if (m_new_line) {
        int i;
        for (i = 0; i < m_indent; i++) {
            m_text += "  ";
        }
        m_new_line = false;
    }
}

void Writer::printf(const char *fmt, ...) {
    va_list ap;
    begin_write();
    va_start(ap, fmt);
    str_vprintf(&m_text, fmt, 1, ap);
    va_end(ap);
}

void Writer::newline_enter() {
    m_indent ++;
    newline();
}

void Writer::newline_exit() {
    m_indent --;
    newline();
}

int Writer::save_tofile(const char *filename)
{
    FILE *f = fopen(filename, "wb");
    if (f) {
        fputs(m_text.text(), f);
        fclose(f);
        return 0;
    }
    return 1;
}

