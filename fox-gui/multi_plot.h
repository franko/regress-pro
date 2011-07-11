#ifndef MULTI_PLOT_H
#define MULTI_PLOT_H

#include <fx.h>
#include <assert.h>

#include <agg2/agg_basics.h>
#include <agg2/agg_array.h>

template <class plot_type, template <unsigned> class layout, unsigned Size>
class plot_array {
  typedef plot_type* plot_ptr;

public:
  plot_array() {
    for (unsigned k = 0; k < Size; k++)
      m_plot[k] = 0;
  }

  void draw(FXDCWindow *dc, int width, int height)
  {
    agg::rect_i r(0, 0, width, height);

    for (unsigned k = 0; k < Size; k++)
      {
	plot_type* plot = m_plot[k];

	if (plot)
	  {
	    agg::rect_i box;
	  
	    m_layout.region_by_id (k, r, box);

	    int w = box.x2 - box.x1, h = box.y2 - box.y1;
	    int x = box.x1, y = box.y1;

	    plot->draw(dc, w, h, x, y);
	  }
      }
  }

  unsigned size() const { return Size; }
  const plot_ptr& operator [] (unsigned i) const { return m_plot[i]; }
        plot_ptr& operator [] (unsigned i)       { return m_plot[i]; }
  const plot_ptr& at(unsigned i) const           { return m_plot[i]; }
        plot_ptr& at(unsigned i)                 { return m_plot[i]; }

private:
  layout<Size> m_layout;
  agg::pod_auto_array<plot_ptr, Size> m_plot;
};

template <unsigned Size>
class vertical_layout {
public:
  vertical_layout() {}

  void region_by_id (unsigned k, const agg::rect_i& r, agg::rect_i& box)
  {
    int h = (r.y2 - r.y1) / Size;
    box.init(r.x1, r.y1 + k * h, r.x2, r.y1 + (k+1) * h);
  }
};

#endif
