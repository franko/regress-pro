/*
  $Id: elliss.h,v 1.8 2006/10/30 22:55:13 francesco Exp $
*/

#ifndef ELLISS_H
#define ELLISS_H

#include "common.h"
#include "cmpl.h"

enum se_type {
  SE_ALPHA_BETA = 0,
  SE_PSI_DEL,
};

#include <gsl/gsl_vector.h>

typedef enum {
  POL_S = 0,
  POL_P = 1
} polar_t;

struct elliss_ab {
  double alpha;
  double beta;
};

typedef struct elliss_ab  ell_ab_t[1];
typedef struct elliss_ab *ell_ab_ptr;

#define DEGREE(d) ((d) * M_PI / 180.0)

extern void
mult_layer_se_jacob (enum se_type type,
		     size_t nb, const cmpl ns[], double phi0, 
		     const double ds[], double lambda,
		     double anlz, ell_ab_t e,
		     gsl_vector *jacob_th, cmpl_vector *jacob_n);

#endif
