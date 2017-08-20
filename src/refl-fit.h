#ifndef REFL_FIT_H
#define REFL_FIT_H

#include "common.h"
#include "spectra.h"
#include "fit-engine.h"

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

extern int          refl_fit_fdf(const gsl_vector *x, void *params,
                                 gsl_vector *f, gsl_matrix * jac);
extern int          refl_fit_f(const gsl_vector *x, void *params,
                               gsl_vector * f);
extern int          refl_fit_df(const gsl_vector *x,
                                void *params, gsl_matrix *jacob);

#endif
