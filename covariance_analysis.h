#ifndef COVARIANCE_ANALYSIS_H
#define COVARIANCE_ANALYSIS_H

#include <gsl/gsl_vector.h>

#include "str.h"

extern void print_covar_analysis(str_t msg, gsl_matrix *covar);

#endif
