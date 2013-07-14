
/*
  $Id$
*/

#ifndef ELLIS_MULTIFIT_H
#define ELLIS_MULTIFIT_H

#include "defs.h"

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

__BEGIN_DECLS

extern int      elliss_multifit_fdf(const gsl_vector *x, void *params,
                                    gsl_vector *f, gsl_matrix * jacob);
extern int      elliss_multifit_f(const gsl_vector *x, void *params,
                                  gsl_vector * f);
extern int      elliss_multifit_df(const gsl_vector *x,
                                   void *params, gsl_matrix *jacob);

__END_DECLS

#endif
