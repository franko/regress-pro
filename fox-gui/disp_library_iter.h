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
    disp_list *m_list;
    disp_node *m_node;

public:
    disp_library_iter(disp_list *list): m_list(list), m_node(list->first) { }

    void reset() {
        m_node = m_list->first;
    }

    disp_t *next_disp()  {
        if (m_node) {
            disp_t *d = m_node->content;
            m_node = m_node->next;
            return d;
        }
        return NULL;
    }

    virtual disp_t* get(int index) {
        return disp_list_get_by_index(m_list, index);
    }
};

#endif
