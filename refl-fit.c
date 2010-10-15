/* 
  $Id: refl-fit.c,v 1.7 2006/07/12 22:48:55 francesco Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <assert.h>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

#include "refl-fit.h"
#include "refl-kernel.h"
#include "fit-engine.h"
#include "refl-get-jacobian.h"

double
get_parameter_jacob_r (fit_param_t const *fp, stack_t const *stack, 
		       struct deriv_info *ideriv, double lambda,
		       gsl_vector *stack_jacob_th,
		       gsl_vector *stack_jacob_n,
		       double rmult, double r_raw)
{
  double result;
  int lyr = fp->layer_nb;

  switch (fp->id)
    {
      double drdth;
      struct { double re, im; } drdn;
      double dnr, dni;

    case PID_FIRSTMUL:
      result = r_raw;
      break;
    case PID_THICKNESS:
      drdth = gsl_vector_get (stack_jacob_th, lyr - 1);
      result = rmult * drdth;
      break;
    case PID_LAYER_N:
      get_model_param_deriv (stack->disp[lyr], &ideriv[lyr],
			     fp, lambda, &dnr, &dni);

      drdn.re = gsl_vector_get (stack_jacob_n, lyr);
      drdn.im = gsl_vector_get (stack_jacob_n, stack->nb + lyr);

      result = rmult * (dnr * drdn.re + dni * drdn.im);
      break;
    default:
      assert (0);
    }

  return result;
}

int
refl_fit_fdf (const gsl_vector *x, void *params, 
              gsl_vector *f, gsl_matrix * jacob)
{
  struct fit_engine *fit = params;
  size_t nb_med = fit->stack->nb;
  struct { double const * ths; cmpl * ns; } actual;
  gsl_vector *r_th_jacob, *r_n_jacob;
  size_t j;

  /* STEP 1 : We apply the actual values of the fit parameters
              to the stack. */

  fit_engine_commit_parameters (fit, x);

  /* STEP 2 : From the stack we retrive the thicknesses and RIs
              informations. */

  actual.ths = stack_get_ths_list (fit->stack);

  r_th_jacob = (jacob ? fit->jac_th : NULL);
  r_n_jacob  = (jacob ? fit->jac_n.refl : NULL);

  for (j = 0; j < spectra_points (fit->spectr); j++)
    {
      float const * spectr_data = spectra_get_values (fit->spectr, j);
      const double lambda = spectr_data[0];
      const double r_meas = spectr_data[1];
      double r_raw, r_theory;
      double rmult = fit->extra->rmult;

      if (fit->cache.th_only)
	actual.ns = fit->cache.ns_full_spectr + j * nb_med;
      else
	{
	  actual.ns = fit->cache.ns;
	  stack_get_ns_list (fit->stack, actual.ns, lambda);
	}
      
      /* STEP 3 : We call the procedure mult_layer_refl_ni */

      r_raw = mult_layer_refl_ni (nb_med, actual.ns, actual.ths, lambda,
				  r_th_jacob, r_n_jacob);

      r_theory = rmult * r_raw;
        
      if (f != NULL)
	gsl_vector_set (f, j, r_theory - r_meas);

      if (jacob)
	{
	  size_t kp, ic;
	  struct deriv_info * ideriv = fit->cache.deriv_info;

	  if (! fit->cache.th_only)
	    {
	      for (ic = 0; ic < nb_med; ic++)
		ideriv[ic].is_valid = 0;
	    }

	  for (kp = 0; kp < fit->parameters->number; kp++)
	    {
	      const fit_param_t *fp = fit->parameters->values + kp;
	      double pjac;

	      pjac = get_parameter_jacob_r (fp, fit->stack, ideriv, lambda,
					    r_th_jacob, r_n_jacob,
					    rmult, r_raw);

	      gsl_matrix_set (jacob, j, kp, pjac);
	    }
	}
    }

  return GSL_SUCCESS;
}

int
refl_fit_f (const gsl_vector *x, void *params, gsl_vector * f)
{
  return refl_fit_fdf (x, params, f, NULL);
}

int
refl_fit_df (const gsl_vector *x, void *params, gsl_matrix *jacob)
{
  return refl_fit_fdf (x, params, NULL, jacob);
}
