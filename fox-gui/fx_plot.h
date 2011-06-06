#ifndef FOXGUI_FX_PLOT_H
#define FOXGUI_FX_PLOT_H

#include <fx.h>

#include "units.h"
#include "agg_array.h"
#include "agg_bounding_rect.h"
#include "agg_path_storage.h"

class owner_resource_manager {
public:
  template <class T>
  static void acquire(T* p) { };

  template <class T>
  static void dispose(T* p) { delete p; };
};

template <class vertex_source, class resource_manager>
class fx_plot {

  struct item  {
    vertex_source *vs;
    FXColor color;

    item() : vs(NULL) {};
    item(vertex_source *_vs, FXColor _col = FXRGB(0,0,0))
      : vs(_vs), color(_col) 
    { };
  };
  
  FXFont *m_font;
  FXFont *m_title_font;

  FXColor m_bgcol;
  FXString m_title;

  agg::pod_bvector<item> m_lines;

  bool m_rect_def;
  agg::rect_d m_rect;

  void dispose_elements() {
    unsigned n = m_lines.size();
    for (unsigned i = 0; i < n; i++)
      resource_manager::dispose(m_lines[i].vs);
  };

public:
  fx_plot(FXApp *app, FXColor bg = FXRGB(255,255,255))
    : m_bgcol(bg), m_rect_def(false)
  {
    m_font = new FXFont(app, "helvetica", 8);
    m_title_font = new FXFont(app, "helvetica", 10, FXFont::Bold);
    m_font->create();
    m_title_font->create();
  };

  ~fx_plot() { 
    this->dispose_elements();
    delete m_font;
    delete m_title_font;
  };

  void clear() {
    this->dispose_elements();
    this->m_lines.clear();
  };

  bool is_empty() const { return m_lines.size() == 0; };
  void set_title(const FXString& t) { m_title = t; };
  void add(vertex_source *vs, FXColor col = FXRGB(0,0,0));
  void draw(FXDCWindow *dc, int ww, int hh, int x_offset = 0, int y_offset = 0);
};

template <class VS, class RM>
void fx_plot<VS,RM>::add(VS *vs, FXColor col)
{
  item it(vs, col);
  this->m_lines.add(it);

  agg::rect_d r;
  agg::bounding_rect_single(*vs, 0, &r.x1, &r.y1, &r.x2, &r.y2);

  if (m_rect_def) {
    this->m_rect = agg::unite_rectangles(r, this->m_rect);
  } else {
    this->m_rect = r;
    m_rect_def = true;
  }
}

template <class VS, class RM>
void fx_plot<VS,RM>::draw(FXDCWindow *dc, int ww, int hh, int xoffs, int yoffs)
{
  if (this->is_empty())
    return;

  const int xbord = 20, ybord = 15;
  int lmarg = xbord, rmarg = xbord;
  int tmarg = ybord, bmarg = ybord;
  const int tickw = 4, tickh = 4;
  units xu(m_rect.x1, m_rect.x2, 6.0), yu(m_rect.y1, m_rect.y2, 3.9);

  dc->setForeground(m_bgcol);
  dc->fillRectangle(xoffs, yoffs, ww, hh);

  dc->setForeground(FXRGB(0,0,0));
  dc->setBackground(m_bgcol);

  if (!m_title.empty())
    {
      int thh = m_title_font->getTextHeight("H", 1);
      int tww = m_title_font->getTextWidth(m_title);
      dc->setFont(m_title_font);
      dc->drawText(xoffs + lmarg + (ww - tww)/2, yoffs + tmarg,
		   m_title.text(), m_title.length());
      tmarg += thh;
    }

  int cheight = m_font->getTextHeight("H", 1);

  FXString label;
  int ylabelwidth = 0;
  int jmin, jmax;

  yu.get_units(jmin, jmax);
  for (int j = jmin; j <= jmax; j++)
    {
      yu.get_tick_label(label, j);
      int lw = m_font->getTextWidth(label);
      if (lw > ylabelwidth)
	ylabelwidth = lw;
    }

  lmarg += ylabelwidth + tickw + 4;
  bmarg += cheight + tickh + 2;

  int areaw = ww - (lmarg + rmarg);
  int areah = hh - (tmarg + bmarg);

  dc->setForeground(FXRGB(220,220,220));
  dc->setLineStyle(LINE_ONOFF_DASH);
  for (int j = jmin + 1; j < jmax; j++)
    {
      double ly = yu.get_scaled_value(yu.get_tick_value(j));
      int yt = yoffs + tmarg + areah - (int) (ly * areah);
      dc->drawLine(xoffs + lmarg, yt, xoffs + lmarg + areaw, yt);
    }

  xu.get_units(jmin, jmax);
  for (int j = jmin + 1; j < jmax; j++)
    {
      double lx = xu.get_scaled_value(xu.get_tick_value(j));
      int xt = xoffs + lmarg + (int) (lx * areaw);
      dc->drawLine(xt, tmarg + yoffs,
		   xt, tmarg + areah + yoffs);
    }

  dc->setLineStyle(LINE_SOLID);
  dc->setForeground(FXRGB(0,0,0));

  dc->drawRectangle(xoffs + lmarg, yoffs + tmarg, areaw, areah);

  dc->setFont(m_font);
  yu.get_units(jmin, jmax);
  for (int j = jmin; j <= jmax; j++)
    {
      double ly = yu.get_scaled_value(yu.get_tick_value(j));
      yu.get_tick_label(label, j);
      int yt = yoffs + tmarg + areah - (int) (ly * areah);
      dc->drawText(xoffs + xbord, yt + cheight/2, label.text(), 
		   label.length());
      dc->drawLine(xoffs + lmarg - tickw, yt, xoffs + lmarg, yt);
    }

  xu.get_units(jmin, jmax);
  xu.get_tick_label(label, jmin);
  int xlabelwidth = m_font->getTextWidth(label);
  for (int j = jmin; j <= jmax; j++)
    {
      double lx = xu.get_scaled_value(xu.get_tick_value(j));
      xu.get_tick_label(label, j);
      int xt = xoffs + lmarg + (int) (lx * areaw);
      dc->drawText(xt - xlabelwidth/2, hh - ybord + yoffs,
		   label.text(), label.length());
      dc->drawLine(xt, tmarg + areah + yoffs,
		   xt, tmarg + areah + tickh + yoffs);
    }

  for (unsigned i = 0; i < m_lines.size(); i++) {
    item& it = m_lines[i];
    VS *vs = it.vs;

    int nb = vs->total_vertices();
    FXPoint *fxp = new FXPoint [nb];

    double x, y;
    unsigned cmd;
    unsigned j = 0;
    vs->rewind(0);
    while(!agg::is_stop(cmd = vs->vertex(&x, &y)))
      {
	double sx = xu.get_scaled_value(x);
	double sy = yu.get_scaled_value(y);
	fxp[j].x = xoffs + lmarg + (int)(sx * areaw);
	fxp[j].y = yoffs + tmarg + areah - (int)(sy * areah);
	j += 1;
      }

    dc->setForeground(it.color);
    dc->drawLines(fxp, nb);
    delete [] fxp;
  }
}

typedef fx_plot<agg::path_storage, owner_resource_manager> plot;

#endif
