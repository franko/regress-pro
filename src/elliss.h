
/* elliss.h
 *
 * Copyright (C) 2005-2011 Francesco Abbate
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef ELLISS_H
#define ELLISS_H

#include <gsl/gsl_vector.h>

#include "common.h"
#include "cmpl.h"
#include "acquisition.h"
#include "ellipsometry-decls.h"

enum se_type {
    SE_ALPHA_BETA = 0,
    SE_PSI_DEL,
};

enum { SE_AOI = 0, SE_ANALYZER = 1 };
enum { SE_ALPHA = 0, SE_BETA = 1 };
enum { SE_TANPSI = 0, SE_COSDEL = 1 };

#define SE_ACQ_INDEX(channel, pid) (2 * (pid) + (channel))
#define SE_ACQ_PARAMETERS_NB(se) ((se) == SE_ALPHA_BETA ? 2 : 1)

struct elliss_ab {
    double alpha;
    double beta;
};

typedef struct elliss_ab  ell_ab_t[1];
typedef struct elliss_ab *ell_ab_ptr;

extern void
mult_layer_refl_se(enum se_type se_type,
                   size_t nb, const cmpl ns[],
                   const double ds[], double lambda,
                   const struct acquisition_parameters *acquisition, ell_ab_t e,
                   gsl_vector *jacob_th, cmpl_vector *jacob_n, double *jacob_acquisition);

#endif
