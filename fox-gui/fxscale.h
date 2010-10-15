
/*
  $Id: fxscale.h,v 1.2 2006/07/12 22:49:19 francesco Exp $
*/

#ifndef FOXGUI_FXSCALE_H
#define FOXGUI_FXSCALE_H

#include <list>
#include <fx.h>

#include "scale.h"

struct PlotInfo {
  FXColor linecolor;
  
  PlotInfo() : linecolor(FXRGB(0,0,0)) { };
  PlotInfo(FXColor col = FXRGB(0,0,0)) : linecolor(col) { };
};

class FXDataPlot {
private:
  static FXFont *font;
  static FXFont *titleFont;

  FXColor bgcol;
  FXString title;

  std::list<XYDataSet> plots;
  std::list<PlotInfo> styles;

  struct limits {
    float inf, sup;
  };

  limits x, y;

public:
  FXDataPlot(FXColor bg = FXRGB(255,255,255));

  static void initPlotEngine(FXApp*);
  static void closePlotEngine();

  void setTitle(const FXString&);

  void addPlot(XYDataSet& ds, FXColor col = FXRGB(0,0,0));

  void clear();

  void draw(FXDCWindow *dc, int ww, int hh, int xoff = 0, int yoffs = 0);
};

#endif

