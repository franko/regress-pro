#ifndef DISP_VS_H
#define DISP_VS_H

#include <agg2/agg_basics.h>

#include "cmpl.h"
#include "dispers.h"

template <class Sampling>
class disp_vs {
public:
  disp_vs(const disp_t* d, cmpl::part_e comp, Sampling& samp) : 
    m_disp(d), m_comp(comp), m_sampling(samp), m_index(0)
  {}

  void rewind(unsigned path_id) { m_index = 0; }

  unsigned vertex(double* x, double* y) {
    if (m_index >= m_sampling.size()) 
      return agg::path_cmd_stop;

    cmpl n = n_value (m_disp, m_sampling[m_index]);
    *x = m_sampling[m_index];

    double c = n.data[m_comp];
    *y = (m_comp == cmpl::real_part ? c : -c);

    int cmd = (m_index == 0 ? agg::path_cmd_move_to : agg::path_cmd_line_to);

    m_index ++;
    return cmd;
  }
  
private:
  const disp_t* m_disp;
  cmpl::part_e m_comp;
  Sampling m_sampling;
  unsigned m_index;
};

#endif
