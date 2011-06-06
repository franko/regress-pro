
#include "spectra-path.h"

void
refl_spectrum_path (const struct spectrum *s, agg::path_storage* p)
{
  int n = spectra_points(s);
  for (int j = 0; j < n; j++)
    {
      const float *val = spectra_get_values(s, j);
      p->line_to(double(val[0]), double(val[1]));
    }
}

void
elliss_spectrum_path (const struct spectrum *s, 
		      agg::path_storage* p1, agg::path_storage* p2)
{
  int n = spectra_points(s);
  for (int j = 0; j < n; j++)
    {
      const float *val = spectra_get_values(s, j);
      p1->line_to(double(val[0]), double(val[1]));
      p2->line_to(double(val[0]), double(val[2]));
    }
}
