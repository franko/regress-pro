#ifndef PLOT_CLIPBOARD_H
#define PLOT_CLIPBOARD_H

#include "agg2/agg_array.h"
#include "str_cpp.h"

struct cpair {
    float x, y;
};

typedef agg::pod_bvector<cpair> cpair_table;

/* Hold a vector of pointers (objects) of which it has the
   ownership. The destructor will essentilly call "delete"
   on all the objects. */
template <typename T>
struct vector_objects : agg::pod_bvector<T*> {
    vector_objects() {}
    ~vector_objects() {
        for (unsigned i = 0; i < this->size(); i++) {
            delete this->at(i);
        }
    }
};

struct line_data {
    vector_objects<str> names;
    vector_objects<cpair_table> tables;
    unsigned size() const { return tables.size(); }
};

struct plot_content {
    str title;
    line_data lines;
};

#endif
