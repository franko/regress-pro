#ifndef MINSAMPLING_H
#define MINSAMPLING_H

#include "spectra.h"

// Minimize the number of points in the spectra and return true is successful.
// It may fail if the subsampling gives less points than the minimum.
extern bool table_sample_minimize(struct spectrum *s, float dlmt, int channel_s, int channel_e, int min_points_nb);

#endif
