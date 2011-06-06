
#include "fxscale.h"
#include "agg_array.h"
#include "agg_bounding_rect.h"

template <class VS, class RM>
fx_plot<VS,RM>::fx_plot(FXColor bg) : bgcol(bg)
{
  this->m_font = new FXFont(app, "helvetica", 8);
  this->m_title_font = new FXFont(app, "helvetica", 10, FXFont::Bold);
  this->m_font->create();
  this->m_title_font->create();
}

template <class VS, class RM>
fx_plot<VS,RM>::clear()
{
  this->dispose_elements();
  this->m_lines.clear();
}

template <class VS, class RM>
fx_plot<VS,RM>::add(vertex_source *vs, FXColor col)
{
  item it(vs, col);
  this->m_lines.add(it);

  agg::rect_d r;
  agg::bounding_rect_single(vs, 0, &r.x1, &r.y1, &r.x2, &r.y2);

  this->m_rect = agg::unite_rectangles(r, this->m_rect);
}

template <class VS, class RM>
fx_plot<VS,RM>::draw(FXDCWindow *dc, int ww, int hh, int xoffs, int yoffs)
{
  if (this->is_empty())
    return;

  const int xbord = 20, ybord = 15;
  int lmarg = xbord, rmarg = xbord;
  int tmarg = ybord, bmarg = ybord;
  const int tickw = 4, tickh = 4;
  Units xu(m_rect.x1, m_rect.x2, 6.0), yu(m_rect.y1, m_rect.y2, 3.9);

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

  yu.getUnits(jmin, jmax);
  for (int j = jmin; j <= jmax; j++)
    {
      yu.getTickLabel(label, j);
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
      double ly = yu.getScaledValue(yu.getTickValue(j));
      int yt = yoffs + tmarg + areah - (int) (ly * areah);
      dc->drawLine(xoffs + lmarg, yt, xoffs + lmarg + areaw, yt);
    }

  xu.getUnits(jmin, jmax);
  for (int j = jmin + 1; j < jmax; j++)
    {
      double lx = xu.getScaledValue(xu.getTickValue(j));
      int xt = xoffs + lmarg + (int) (lx * areaw);
      dc->drawLine(xt, tmarg + yoffs,
		   xt, tmarg + areah + yoffs);
    }

  dc->setLineStyle(LINE_SOLID);
  dc->setForeground(FXRGB(0,0,0));

  dc->drawRectangle(xoffs + lmarg, yoffs + tmarg, areaw, areah);

  dc->setFont(m_font);
  yu.getUnits(jmin, jmax);
  for (int j = jmin; j <= jmax; j++)
    {
      double ly = yu.getScaledValue(yu.getTickValue(j));
      yu.getTickLabel(label, j);
      int yt = yoffs + tmarg + areah - (int) (ly * areah);
      dc->drawText(xoffs + xbord, yt + cheight/2, label.text(), 
		   label.length());
      dc->drawLine(xoffs + lmarg - tickw, yt, xoffs + lmarg, yt);
    }

  xu.getUnits(jmin, jmax);
  xu.getTickLabel(label, jmin);
  int xlabelwidth = m_font->getTextWidth(label);
  for (int j = jmin; j <= jmax; j++)
    {
      double lx = xu.getScaledValue(xu.getTickValue(j));
      xu.getTickLabel(label, j);
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
    vs->rewind(0);
    while(!is_stop(cmd = vs->vertex(&x, &y)))
      {
	double sx = xu.getScaledValue(x);
	double sy = yu.getScaledValue(y);
	fxp[j].x = xoffs + lmarg + (int)(sx * areaw);
	fxp[j].y = yoffs + tmarg + areah - (int)(sy * areah);
      }

    dc->setForeground(it.color);
    dc->drawLines(fxp, nb);
    delete [] fxp;
  }
}
