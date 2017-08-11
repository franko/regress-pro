#ifndef MINSAMPLING_H
#define MINSAMPLING_H

#include "spectra.h"

__BEGIN_DECLS

extern void table_sample_minimize(struct spectrum *s, float dlmt, int channel_s, int channel_e);

__END_DECLS

#endif
