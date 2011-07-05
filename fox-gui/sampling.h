#ifndef SAMPLING_H
#define SAMPLING_H

#include "agg2/agg_array.h"

class sampling_unif {
  unsigned m_nb_points;

  double m_start;
  double m_end;
  double m_stride;
  
public:
  sampling_unif(double s_start, double s_end, unsigned nsp) :
    m_nb_points(nsp), m_start(s_start), m_end(s_end)
  {
    m_stride = (m_end - m_start) / (m_nb_points - 1);
  }

  unsigned size() const { return m_nb_points; }

  const double operator [] (unsigned i) const { return m_start + i * m_stride; }
        double operator [] (unsigned i)       { return m_start + i * m_stride; }
  const double at(unsigned i) const           { return m_start + i * m_stride; }
        double at(unsigned i)                 { return m_start + i * m_stride; }
  double value_at(unsigned i) const           { return m_start + i * m_stride; }
};

typedef agg::pod_array<double> sampling_lookup;
  
#endif
