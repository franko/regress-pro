#ifndef DISP_LIBRARY_ITER_H
#define DISP_LIBRARY_ITER_H

#include "dispers-library.h"

struct disp_iterator {
    disp_iterator() {}
    virtual ~disp_iterator() {}
    virtual void reset() = 0;
    virtual disp_t *next_disp() = 0;

    const char *start() {
        reset();
        return next();
    }

    disp_t *start_disp() {
        reset();
        return next_disp();
    }

    const char *next() {
        disp_t *d = next_disp();
        return (d ? CSTR(d->name) : NULL);
    }

    int length() {
        int n = 0;
        for(disp_t *d = start_disp(); d; d = next_disp()) {
            n ++;
        }
        return n;
    }

    virtual disp_t *get(int index) {
        int n = 0;
        for(disp_t *d = start_disp(); d; d = next_disp(), n++) {
            if (n == index) {
                return disp_copy(d);
            }
        }
        return NULL;
    }
};

class disp_library_iter : public disp_iterator {
    int m_lib_iter;

public:
    disp_library_iter() : m_lib_iter(0) { }

    void reset() {
        m_lib_iter = 0;
    }

    disp_t *next_disp()  {
        const char* name;
        return dispers_library_get(m_lib_iter ++, &name);
    }

    virtual disp_t* get(int index) {
        const char* name;
        disp_t *d = dispers_library_get(index, &name);
        return (d ? disp_copy(d) : NULL);
    }
};

class userlib_iter : public disp_iterator {
    disp_node *m_current;
public:
    userlib_iter(): m_current(user_lib->first) { }

    void reset() { m_current = user_lib->first; }

    disp_t *next_disp() {
        if (m_current) {
            disp_t *d = m_current->content;
            m_current = m_current->next;
            return d;
        }
        return NULL;
    }
};

#endif
