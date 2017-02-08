#ifndef REFL_KERNEL_H
#define REFL_KERNEL_H

#include "common.h"
#include "cmpl.h"
#include "spectra.h"
#include "ellipsometry-decls.h"

#include <gsl/gsl_vector.h>

enum { SR_RMULT = 0, SR_BANDWIDTH };

#define SR_ACQ_PARAMETERS_NB 2

/* reflectivity for normal incidence with multi-layer film */
extern int
mult_layer_refl_sr(int nb, const cmpl ns[], const double ds[],
                   double lambda, const struct acquisition_parameters *acquisition,
                   double result[1], double *jacob_th, cmpl *jacob_n, double *jacob_acq);

#endif
