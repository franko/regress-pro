#ifndef SPECTRA_H
#define SPECTRA_H

#include "defs.h"
#include "data-view.h"

__BEGIN_DECLS

enum system_kind {
  SYSTEM_UNDEFINED = 0,
  SYSTEM_REFLECTOMETER,
  SYSTEM_ELLISS_AB,
  SYSTEM_ELLISS_PSIDEL,
  SYSTEM_EXCEED_VALUE
};

struct spectral_range {
  int active;
  float min, max;
};

struct system_config {
  enum system_kind system;
  /* Ellipsometry parameters: */
  double aoi;
  double analyzer;
  double numap;
};

struct spectrum {
  struct system_config config;
  struct data_view table[1];
};

#define spectra_points(s) ((s)->table->rows)

typedef struct spectrum spectr_t[1];
typedef struct spectrum *spectr_ptr;

extern struct spectrum * spectra_copy        (struct spectrum *s);
extern struct spectrum * spectra_alloc       (struct spectrum *s);
extern void              spectra_free        (struct spectrum *s);

extern float             get_lambda_by_index (struct spectrum *s, int idx);

extern float const *     spectra_get_values  (struct spectrum const *s, int j);

extern struct spectrum * load_gener_spectrum (const char *filename);

extern void              spectr_cut_range    (struct spectrum *s,
					      float inf, float sup);


__END_DECLS

#endif
