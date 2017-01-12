#ifndef REFL_KERNEL_H
#define REFL_KERNEL_H

#include "common.h"
#include "cmpl.h"
#include "spectra.h"
#include "ellipsometry-decls.h"

#include <gsl/gsl_vector.h>

/* reflectivity for normal incidence with multi-layer film */
extern double
mult_layer_refl_sr(size_t nb, const cmpl ns[], const double ds[],
                   double lambda, const struct acquisition_parameters *acquisition,
                   gsl_vector *jacob_th, gsl_vector *jacob_n, double *jacob_acq);

#endif
