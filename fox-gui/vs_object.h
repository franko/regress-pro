#ifndef VS_OBJECT_H
#define VS_OBJECT_H

#include "defs.h"

#include "agg_trans_affine.h"
#include "agg_conv_transform.h"

class owner_management {
public:
    template <class T>
    static void acquire(T* p) { };

    template <class T>
    static void dispose(T* p) {
        delete p;
    };
};

struct vs_object {
    virtual void rewind(unsigned path_id) = 0;
    virtual unsigned vertex(double* x, double* y) = 0;
    virtual void apply_transform(const agg::trans_affine& m, double as) = 0;
    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2) = 0;
    virtual ~vs_object() { }
};

template <class VertexSource>
class vs_scaling_gen : public vs_object {
    typedef VertexSource base_type;
public:
    template <class InitType>
    vs_scaling_gen(InitType init0) :
        m_source(init0), m_mtx(), m_trans(m_source, m_mtx)
    {}

    template <class InitType>
    vs_scaling_gen(InitType init0, int index) :
        m_source(init0, index), m_mtx(), m_trans(m_source, m_mtx)
    {}

    virtual void rewind(unsigned path_id) {
        m_source.rewind(path_id);
    }

    virtual unsigned vertex(double* x, double* y) {
        return m_trans.vertex(x, y);
    }

    virtual void apply_transform(const agg::trans_affine& m, double as) {
        m_mtx = m;
    }

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2) {
        agg::bounding_rect_single(m_source, 0, x1, y1, x2, y2);
    }

private:
    base_type m_source;
    agg::trans_affine m_mtx;
    agg::conv_transform<base_type> m_trans;
};

#endif
