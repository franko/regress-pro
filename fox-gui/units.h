
/*
  $Id: units.h,v 1.2 2006/07/12 22:49:19 francesco Exp $
*/

#ifndef FOXGUI_UNITS_H
#define FOXGUI_UNITS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <fxver.h>
#include <fxdefs.h>

#include <FXString.h>

class Units {
private:
  int major;
  int order;
  double dmajor; // equal to (major * 10^order)
  int inf, sup; // expressed in the base of (major * 10^order)
  int nb_decimals;

 private:
  void init(double min, double max, double spacefact);

public:
  Units (int nb, double x[], double spacefact = 5.0);
  Units (double min, double max, double spacefact = 5.0)
    { init(min, max, spacefact); };

  void   getUnits(int &start, int &fin) { start = inf; fin = sup; };
  void   getUnits(int &start, int &fin, double &step) 
  { start = inf; fin = sup; step = dmajor; };

  int    getDecimals () const { return nb_decimals; };
  void   getTickLabel (FX::FXString &lab, int tick) const;
  double getTickValue (int tick) const { return dmajor * tick; };

  double getScaledValue(double x);
};

extern void get_limits (int nb, double x[], double &inf, double &sup);

#endif
