#include "common.h"
#include "writer.h"

Writer::Writer() : m_indent(0), m_new_line(true) {
    str_init(m_text, 1024);
};

void Writer::begin_write()
{
    if (m_new_line) {
        int i;
        for (i = 0; i < m_indent; i++) {
            str_append_c(m_text, "  ", 0);
        }
        m_new_line = false;
    }
}

Writer::~Writer() {
    str_free(m_text);
}

void Writer::printf(const char *fmt, ...) {
    va_list ap;
    begin_write();
    va_start(ap, fmt);
    str_vprintf(m_text, fmt, 1, ap);
    va_end(ap);
}

void Writer::newline() {
    str_append_c(m_text, "\n", 0);
    m_new_line = 1;
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
        fputs(CSTR(m_text), f);
        fclose(f);
        return 0;
    }
    return 1;
}
