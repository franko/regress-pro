
/*
  $Id: scale.h,v 1.2 2006/07/12 22:49:19 francesco Exp $
*/

#ifndef FOXGUI_SCALE_H
#define FOXGUI_SCALE_H

#include "units.h"
#include "data-view.h"

struct Point {
  float x, y;
};

class XYDataSet {
private:
  struct data_view data[1];
  int column_x;
  int column_y;

  void init(const struct data_view *data, int column_x, int column_y);

public:
  XYDataSet();
  XYDataSet(const struct data_view *data, int column_x, int column_y);
  XYDataSet(const XYDataSet &);
  ~XYDataSet () { data_view_dealloc (this->data); };

  XYDataSet& operator= (const XYDataSet& s);

  void getLimits(Point &bl, Point &tr) const;
  int pointsNumber() const { return this->data->rows; };
  Point getPoint(int idx) const;
};

#endif
