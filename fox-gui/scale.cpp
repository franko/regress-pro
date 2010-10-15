
#include "scale.h"

void
XYDataSet::getLimits(Point &bl, Point &tr) const
{
  int nb = this->pointsNumber();

  if (nb <= 0) return;

  bl = getPoint(0);
  tr = getPoint(0);

  for (int j = 1; j < nb; j++)
    {
      Point p = getPoint(j);

      if (bl.x > p.x) bl.x = p.x;
      if (tr.x < p.x) tr.x = p.x;
      if (bl.y > p.y) bl.y = p.y;
      if (tr.y < p.y) tr.y = p.y;
    }
}

void
XYDataSet::init (const struct data_view *tb, int cx, int cy)
{
  data_view_copy (this->data, tb);

  this->column_x = cx;
  this->column_y = cy;
}

XYDataSet::XYDataSet ()
{
  data_view_init_empty (this->data);

  this->column_x = -1;
  this->column_y = -1;
}

XYDataSet::XYDataSet (const struct data_view *dw, int cx, int cy)
{
  init (dw, cx, cy);
}

XYDataSet::XYDataSet (const XYDataSet& s)
{
  init (s.data, s.column_x, s.column_y);
}

XYDataSet&
XYDataSet::operator= (const XYDataSet& s)
{
  if (this != &s)
    {
      data_view_dealloc (this->data);
      init (s.data, s.column_x, s.column_y);
    }
  return *this;
}

Point
XYDataSet::getPoint(int idx) const
{
  Point p;
  p.x = data_view_get (this->data, idx, this->column_x);
  p.y = data_view_get (this->data, idx, this->column_y);
  return p;
}
