#ifndef SPECTRUM_PLOT_H
#define SPECTRUM_PLOT_H

#include "spectra.h"
#include "plot_canvas.h"

extern void spectra_plot_simple(plot_canvas* canvas, struct spectrum* spectr);

extern void spectra_plot(plot_canvas* canvas, struct spectrum* ref_spectr,
                         struct spectrum *mod_spectr);

#endif
