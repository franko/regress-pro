
#include "spectra-path.h"
#include "fit-engine.h"

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

void
refl_spectra_plot (struct fit_engine *fit, plot* p)
{
  struct spectrum *gensp = generate_spectrum (fit);

  agg::path_storage *esp = new agg::path_storage;
  agg::path_storage *gsp = new agg::path_storage;
  refl_spectrum_path (fit->spectr, esp);
  refl_spectrum_path (gensp, gsp);

  p->clear();
  p->add(esp, FXRGB(255,0,0));
  p->add(gsp);
  p->set_title("Reflectivity");

  spectra_free (gensp);
}

void
elliss_spectra_plot (struct fit_engine *fit, plot* p1, plot* p2)
{
  struct spectrum *gensp = generate_spectrum (fit);

  agg::path_storage *esp1 = new agg::path_storage;
  agg::path_storage *esp2 = new agg::path_storage;
  agg::path_storage *gsp1 = new agg::path_storage;
  agg::path_storage *gsp2 = new agg::path_storage;

  elliss_spectrum_path (fit->spectr, esp1, esp2);
  elliss_spectrum_path (gensp, gsp1, gsp2);

  p1->clear();
  p1->add(esp1, FXRGB(255,0,0));
  p1->add(gsp1);
  p1->set_title(fit->system_kind == SYSTEM_ELLISS_AB ? "SE alpha" : "Tan(Psi)");

  p2->clear();
  p2->add(esp2, FXRGB(255,0,0));
  p2->add(gsp2);
  p2->set_title(fit->system_kind == SYSTEM_ELLISS_AB ? "SE beta" : "Cos(Delta)");

  spectra_free (gensp);
}
