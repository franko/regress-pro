#ifndef AGG_PLOT_ARRAY_H
#define AGG_PLOT_ARRAY_H

#include <agg2/agg_array.h>
#include <agg2/agg_trans_affine.h>

#include "canvas.h"

namespace newplot {
template <class Plot, class Layout>
class plot_array {
public:
    plot_array() { }

    virtual ~plot_array() {
        clear();
    }

    void clear() {
        for(unsigned i = 0; i < m_plot.size(); i++) {
            delete m_plot[i];
        }

        m_plot.clear();
        m_layout.reset();
    }

    unsigned size() const {
        return m_plot.size();
    }

    Plot* plot(unsigned i) {
        return m_plot[i];
    }

    void add(Plot* plot) {
        m_plot.add(plot);
        m_layout.add();
    }

    void draw(canvas* canvas, int width, int height);

private:
    agg::pod_bvector<Plot*> m_plot;
    Layout m_layout;
};

template <class Plot, class Layout>
void plot_array<Plot, Layout>::draw(canvas* canvas, int width, int height)
{
    agg::trans_affine mt;
    for(unsigned i = 0; i < m_plot.size(); i++) {
        Plot* plot = m_plot[i];
        m_layout.get_matrix(mt, width, height, i);
        plot->draw(*canvas, mt);
    }
}

class vertical_layout {
public:
    vertical_layout() : m_size(0) {}

    void add() {
        m_size++;
    }
    void reset() {
        m_size = 0;
    }

    void get_matrix(agg::trans_affine& mt, int width, int height, unsigned i) {
        unsigned n = m_size;
        if(n > 0) {
            double w = width, h = height / n;
            mt.sx  = w;
            mt.shy = 0.0;
            mt.shx = 0.0;
            mt.sy  = h;
            mt.tx  = 0.0;
            mt.ty  = h * (n-i-1);
        } else {
            mt.reset();
        }
    }

private:
    unsigned m_size;
};
}

#endif
