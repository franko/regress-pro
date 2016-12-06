#ifndef SPECTRA_H
#define SPECTRA_H

#include "defs.h"
#include "data-view.h"
#include "acquisition.h"

__BEGIN_DECLS

struct spectral_range {
    int active;
    float min, max;
};

struct spectrum {
    struct acquisition_parameters acquisition[1];
    struct data_view table[1];
};

#define spectra_points(s) ((s)->table->rows)

typedef struct spectrum spectr_t[1];
typedef struct spectrum *spectr_ptr;

extern struct spectrum * spectra_copy(struct spectrum *s);
extern struct spectrum * spectra_alloc(struct spectrum *s);
extern void              spectra_free(struct spectrum *s);
extern void              spectra_resize(struct spectrum *s, int nr);
extern void              spectra_wavelength_range(struct spectrum *s, double *wl_min, double *wl_max);
extern float             get_lambda_by_index(struct spectrum *s, int idx);

extern float const *     spectra_get_values(struct spectrum const *s, int j);

extern struct spectrum * load_gener_spectrum(const char *filename, str_ptr *error_msg);

extern void              spectr_cut_range(struct spectrum *s, float inf, float sup);

__END_DECLS

#endif
