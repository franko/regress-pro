#ifndef SPECTRA_PATH_H
#define SPECTRA_PATH_H

#include "agg2/agg_path_storage.h"

#include "spectra.h"
#include "dispers.h"
#include "fx_plot.h"

extern void refl_spectrum_path (const struct spectrum *s, agg::path_storage* p);
extern void elliss_spectrum_path (const struct spectrum *s, 
				  agg::path_storage* p1, agg::path_storage* p2);

extern void refl_spectra_plot   (struct fit_engine *fit, plot *p);
extern void elliss_spectra_plot (struct fit_engine *fit, plot *p1, plot *p2);


template <class sampling>
void disp_path (const disp_t *disp, sampling& samp, agg::path_storage* p1, agg::path_storage* p2)
{
  for (unsigned j = 0; j < samp.size(); j++)
    {
      double wl = samp[j];

      cmpl n = n_value (disp, wl);
      double r = creal(n), i = -cimag(n);

      if (j == 0)
	{
	  p1->move_to(wl, r);
	  p2->move_to(wl, i);
	}
      else
	{
	  p1->line_to(wl, r);
	  p2->line_to(wl, i);
	}
    }
}

template <class sampling>
void disp_plot (const disp_t *ref_disp, const disp_t *mod_disp, 
		sampling& samp, plot* p1, plot* p2)
{
  agg::path_storage *r0 = new agg::path_storage;
  agg::path_storage *m0 = new agg::path_storage;
  agg::path_storage *r1 = new agg::path_storage;
  agg::path_storage *m1 = new agg::path_storage;

  disp_path (ref_disp, samp, r0, r1);
  disp_path (mod_disp, samp, m0, m1);

  p1->clear();
  p1->add(r0, FXRGB(255,0,0));
  p1->add(m0);
  p1->set_title("refractive index");

  p2->clear();
  p2->add(r1, FXRGB(255,0,0));
  p2->add(m1);
  p2->set_title("abs coefficient");
}

#endif
