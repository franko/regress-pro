#ifndef DISP_LIBRARY_ITER_H
#define DISP_LIBRARY_ITER_H

#include "dispers-library.h"

class disp_library_iter {
    int m_lib_iter;

public:
    disp_library_iter() : m_lib_iter(0) { }

    void reset() {
        m_lib_iter = 0;
    }

    const char *start() {
        reset();
        return next();
    }

    const char *next()  {
        const char* name;
        if(dispers_library_get(m_lib_iter ++, &name)) {
            return name;
        }
        return NULL;
    }

    disp_t* get(const char* name) {
        return dispers_library_search(name);
    }

    int length() {
        int n = 0;
        for(const char *nm = start(); nm; nm = next()) {
            n ++;
        }
        return n;
    }
};

#endif
