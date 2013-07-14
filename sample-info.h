
/*
  $Id$
 */

#ifndef SAMPLE_INFO_H
#define SAMPLE_INFO_H

#include "str.h"
#include "spectra.h"
#include "stack.h"

__BEGIN_DECLS

struct sample_info {
    str_t spectrum_name;

    double *constraints;
    int constr_nb;

    double *individual;
    int indiv_nb;
};

struct multi_fit_info {
    int samples_number;
    struct spectrum **spectra_list;

    struct strategy constraints;
    struct strategy individual;
};

extern struct sample_info * sample_info_new(int constr_nb, int indiv_nb);
extern struct sample_info * sample_info_copy(struct sample_info const *s);
extern void                 sample_info_free(struct sample_info *sample);

extern struct multi_fit_info * multi_fit_info_new(int samples_number);
extern void                    multi_fit_info_free(struct multi_fit_info *i);

__END_DECLS

#endif
