#ifndef ELLIS_FIT_H
#define ELLIS_FIT_H

#include "defs.h"

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

extern int      elliss_fit_fdf(const gsl_vector *x, void *params,
                               gsl_vector *f, gsl_matrix * jacob);
extern int      elliss_fit_f(const gsl_vector *x, void *params,
                             gsl_vector * f);
extern int      elliss_fit_df(const gsl_vector *x,
                              void *params, gsl_matrix *jacob);

#endif
