#ifndef DISP_VS_H
#define DISP_VS_H

#include <agg2/agg_basics.h>

#include "cmpl.h"
#include "dispers.h"
#include "vs_object.h"

template <class Sampling>
class disp_base_vs {
public:
    disp_base_vs(const disp_t* d, cmpl::part_e comp, Sampling& samp) :
        m_disp(d), m_comp(comp), m_sampling(samp), m_index(0)
    {}

    void rewind(unsigned path_id) {
        m_index = 0;
    }

    unsigned vertex(double* x, double* y) {
        if(m_index >= m_sampling.size()) {
            return agg::path_cmd_stop;
        }

        cmpl n = n_value(m_disp, m_sampling[m_index]);
        *x = m_sampling[m_index];

        double c = n.data[m_comp];
        *y = (m_comp == cmpl::real_part ? c : -c);

        return (m_index++ == 0 ? agg::path_cmd_move_to : agg::path_cmd_line_to);
    }

private:
    const disp_t* m_disp;
    cmpl::part_e m_comp;
    Sampling& m_sampling;
    unsigned m_index;
};

template <class Sampling>
class disp_vs : public vs_object {
    typedef disp_base_vs<Sampling> base_type;
public:
    disp_vs(const disp_t* d, cmpl::part_e comp, Sampling& samp) :
        m_source(d, comp, samp), m_mtx(), m_trans(m_source, m_mtx)
    {}

    virtual void rewind(unsigned path_id) {
        m_source.rewind(path_id);
    }

    virtual unsigned vertex(double* x, double* y) {
        return m_trans.vertex(x, y);
    }

    virtual void apply_transform(const agg::trans_affine& m, double as) {
        m_trans.transformer(m);
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
