#ifndef SPECTRA_PATH_H
#define SPECTRA_PATH_H

#include "spectra.h"
#include "agg_path_storage.h"

extern void refl_spectrum_path (const struct spectrum *s, agg::path_storage* p);
extern void elliss_spectrum_path (const struct spectrum *s, 
				  agg::path_storage* p1, agg::path_storage* p2);

#endif
