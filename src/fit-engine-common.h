#ifndef FIT_ENGINE_COMMON_H
#define FIT_ENGINE_COMMON_H

#include "defs.h"
#include "cmpl.h"
#include "dispers.h"

__BEGIN_DECLS

struct stack_cache {
    int is_valid;
    int nb_med;
    short int th_only;
    short int need_aoi_derivative;
    cmpl *ns;
    struct deriv_info * deriv_info;
    cmpl *ns_full_spectr;
};

struct fit_config {
    double chisq_threshold;
    int threshold_given;
    int nb_max_iters;
    int subsampling;
    struct spectral_range spectr_range;
    double epsabs, epsrel;
};

__END_DECLS

#endif
