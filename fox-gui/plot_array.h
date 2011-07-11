#ifndef PLOT_ARRAY_H
#define PLOT_ARRAY_H

#include <fx.h>
#include <assert.h>

#include <agg2/agg_basics.h>
#include <agg2/agg_array.h>

template <class plot_type, class layout_type, unsigned Size>
class plot_auto_array : public agg::pod_auto_array<plot_type*, Size> {
public:
  plot_auto_array() : m_layout(Size) { }

  layout_type& layout() { return m_layout; }

private:
  layout_type m_layout;
};

template <class plot_type, class layout_type, unsigned Size>
class plot_auto_vector : public agg::pod_auto_vector<plot_type*, Size> {
public:
  plot_auto_vector() { }

  layout_type& layout() { return m_layout; }

private:
  layout_type m_layout;
};

template <class plot_array>
void draw(plot_array& plot, FXDCWindow *dc, int width, int height)
{
  agg::rect_i r(0, 0, width, height);
  agg::rect_i box;

  for (unsigned k = 0; k < plot.size(); k++)
    {
      if (plot[k])
	{
	  plot.layout().region_by_id (k, r, box);
	  plot[k]->draw(dc, box);
	}
    }
}

class vertical_layout {
public:
  vertical_layout(unsigned n = 0) : m_size(n) {}

  void resize(unsigned n) { m_size = n; }

  void region_by_id (unsigned k, const agg::rect_i& r, agg::rect_i& box)
  {
    if (m_size > 0)
      {
	int h = (r.y2 - r.y1) / m_size;
	box.init(r.x1, r.y1 + k * h, r.x2, r.y1 + (k+1) * h);
      }
  }

private:
  unsigned m_size;
};

#endif
