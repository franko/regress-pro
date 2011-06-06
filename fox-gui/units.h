#ifndef FOXGUI_UNITS_H
#define FOXGUI_UNITS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <fxver.h>
#include <fxdefs.h>

#include <FXString.h>

class units {
private:
  int major;
  int order;
  double dmajor; // equal to (major * 10^order)
  int inf, sup; // expressed in the base of (major * 10^order)
  int nb_decimals;

 private:
  void init(double min, double max, double spacefact);

public:
  units (int nb, double x[], double spacefact = 5.0);
  units (double min, double max, double spacefact = 5.0)
    { init(min, max, spacefact); };

  void   get_units(int &start, int &fin) { start = inf; fin = sup; };
  void   get_units(int &start, int &fin, double &step) 
  { start = inf; fin = sup; step = dmajor; };

  int    get_decimals () const { return nb_decimals; };
  void   get_tick_label (FX::FXString &lab, int tick) const;
  double get_tick_value (int tick) const { return dmajor * tick; };

  double get_scaled_value(double x);
};

extern void get_limits (int nb, double x[], double &inf, double &sup);

#endif
