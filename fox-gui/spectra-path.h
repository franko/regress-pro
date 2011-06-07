#ifndef SPECTRA_PATH_H
#define SPECTRA_PATH_H

#include "spectra.h"
#include "agg_path_storage.h"
#include "fx_plot.h"

extern void refl_spectrum_path (const struct spectrum *s, agg::path_storage* p);
extern void elliss_spectrum_path (const struct spectrum *s, 
				  agg::path_storage* p1, agg::path_storage* p2);

extern void refl_spectra_plot   (struct fit_engine *fit, plot *p);
extern void elliss_spectra_plot (struct fit_engine *fit, plot *p1, plot *p2);

#endif
