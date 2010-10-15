
/*
  $Id: fxscale.cpp,v 1.2 2006/07/12 22:49:19 francesco Exp $
*/

#include "fxscale.h"

FXFont *FXDataPlot::font = NULL;
FXFont *FXDataPlot::titleFont = NULL;

FXDataPlot::FXDataPlot(FXColor bg)
  : bgcol(bg)
{
}

void
FXDataPlot::initPlotEngine(FXApp *app)
{
  FXDataPlot::font = new FXFont(app, "helvetica", 8);
  FXDataPlot::titleFont = new FXFont(app, "helvetica", 10, FXFont::Bold);
  font->create();
  titleFont->create();
}

void
FXDataPlot::closePlotEngine()
{
  delete FXDataPlot::font;
  delete FXDataPlot::titleFont;
}

void
FXDataPlot::setTitle(const FXString &tt)
{
  title = tt;
}

void
FXDataPlot::clear()
{
  plots.clear();
  styles.clear();
}

void
FXDataPlot::addPlot(XYDataSet& ds, FXColor col)
{
  PlotInfo style(col);

  plots.push_back(ds);
  styles.push_back(style);

  Point bl, tr;
  ds.getLimits(bl, tr);
  if (plots.size() == 1)
    {
      x.inf = bl.x;
      x.sup = tr.x;
      y.inf = bl.y;
      y.sup = tr.y;
    }
  else
    {
      if (bl.x < x.inf) x.inf = bl.x;
      if (tr.x > x.sup) x.sup = tr.x;
      if (bl.y < y.inf) y.inf = bl.y;
      if (tr.y > y.sup) y.sup = tr.y;
    }
}

void
FXDataPlot::draw(FXDCWindow *dc, int ww, int hh, int xoffs, int yoffs)
{
  if (plots.size() == 0)
    return;

  const int xbord = 20, ybord = 15;
  int lmarg = xbord, rmarg = xbord;
  int tmarg = ybord, bmarg = ybord;
  const int tickw = 4, tickh = 4;
  Units xu(x.inf, x.sup, 6.0), yu(y.inf, y.sup, 3.9);

  dc->setForeground(bgcol);
  dc->fillRectangle(xoffs, yoffs, ww, hh);

  dc->setForeground(FXRGB(0,0,0));
  dc->setBackground(bgcol);

  if (!title.empty())
    {
      int thh = titleFont->getTextHeight("H", 1);
      int tww = titleFont->getTextWidth(title);
      dc->setFont(titleFont);
      dc->drawText(xoffs + lmarg + (ww - tww)/2, yoffs + tmarg,
		   title.text(), title.length());
      tmarg += thh;
    }

  int cheight = font->getTextHeight("H", 1);

  FXString label;
  int ylabelwidth = 0;
  int jmin, jmax;

  yu.getUnits(jmin, jmax);
  for (int j = jmin; j <= jmax; j++)
    {
      yu.getTickLabel(label, j);
      int lw = font->getTextWidth(label);
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

  dc->setFont(font);
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
  int xlabelwidth = font->getTextWidth(label);
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

  std::list<XYDataSet>::iterator ds = plots.begin();
  std::list<PlotInfo>::iterator style = styles.begin();
  for ( /* */; ds != plots.end(); ds++, style++)
    {
      int nb = ds->pointsNumber();
      FXPoint *fxp = new FXPoint [nb];
      for (int j = 0; j < nb; j++)
	{
	  Point ps, pu = ds->getPoint(j);
	  ps.x = xu.getScaledValue(pu.x);
	  ps.y = yu.getScaledValue(pu.y);
	  fxp[j].x = xoffs + lmarg + (int)(ps.x * areaw);
	  fxp[j].y = yoffs + tmarg + areah - (int)(ps.y * areah);
	}
      dc->setForeground(style->linecolor);
      dc->drawLines(fxp, nb);
      delete [] fxp;
    }
}
