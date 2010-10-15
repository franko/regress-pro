
#include <math.h>

#include "units.h"

void
get_limits (int nb, double x[], double &inf, double &sup)
{
  if (nb <= 0)
    return;
  inf = sup = x[0];
  for (int j = 1; j < nb; j++)
    {
      if (inf > x[j])
	inf = x[j];
      if (sup < x[j])
	sup = x[j];
    }
}

void
Units::init (double yinf, double ysup, double spacefact)
{
  double del;

  if (ysup == yinf)
    ysup = yinf + 1.0;

  del = (ysup - yinf) / spacefact;

  order = (int) floor(log10(del));

  double expf = pow(10, order);
  double delr = del / expf;

  if (5 <= delr)
    major = 5;
  else if (2 <= delr)
    major = 2;
  else
    major = 1;

  inf = (int) floor(yinf / (major * expf));
  sup = (int) ceil(ysup / (major * expf));

  nb_decimals = (order < 0 ? -order : 0);

  dmajor = major * expf;
}

Units::Units (int nb, double y[], double spacefact)
{
  double yinf, ysup;
  get_limits (nb, y, yinf, ysup);
  init (yinf, ysup, spacefact);
}

void
Units::getTickLabel (FX::FXString &lab, int tick) const
{
  bool minus = (inf < 0);
  int asup = (minus ? -inf : sup);
  char fmt[8];

  if (nb_decimals == 0)
    {
      int space = (int)log10(asup * dmajor) + (minus ? 1 : 0) + 1;
      sprintf (fmt, "%%%id", space);
      lab.format (fmt, (int) (tick * dmajor));
    }
  else
    {
      int dec = (nb_decimals < 10 ? nb_decimals : 9);
      int base = floor(asup * dmajor);
      int space = dec + (base > 0 ? (int)log10(base): 0) + 1 \
	+ (minus ? 1 : 0) + 1;
      sprintf (fmt, "%%%i.%if", space, dec);
      lab.format (fmt, tick * dmajor);
    }
}

double
Units::getScaledValue(double x)
{
  int inf, sup;
  double ef;

  getUnits(inf, sup, ef);
  return (x - inf * ef) / ((sup - inf) * ef);
}
