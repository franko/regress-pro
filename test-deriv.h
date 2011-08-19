#ifndef TEST_DERIV_H
#define TEST_DERIV_H

#ifdef DEBUG_REGRESS

#include "elliss.h"

void
test_elliss_deriv (enum se_type spkind,
		   size_t _nb, const cmpl ns[], double phi0, 
		   const double ds[], double lambda,
		   double anlz);

#endif

#endif
