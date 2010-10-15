
/*
  $Id: refl-kernel.c,v 1.2 2006/07/12 22:48:55 francesco Exp $
*/

#include <assert.h>

#include "refl-kernel.h"

static inline cmpl
csqr(cmpl x)
{
  return x*x;
}

static inline cmpl
refl_coeff_ni (cmpl nt, cmpl nb)
{
  return (nb - nt) / (nb + nt);
}

static cmpl
refl_coeff_ext_ni (cmpl nt, cmpl nb, cmpl * drdnt, cmpl * drdnb)
{
  cmpl aux = 1 / (nb + nt); 
  cmpl r = (nb - nt) * aux;
  aux *= aux;
  *drdnt = - 2.0 * nb * aux;
  *drdnb =   2.0 * nt * aux;
  return r;
}

static cmpl
mult_layer_refl_ni_nojacob (int nb, const cmpl ns[], const double ds[],
			    double lambda)
{
  const double omega = 2 * M_PI / lambda;
  cmpl nt, nc, R;
  int j;

  nt = ns[nb-2];
  nc = ns[nb-1];

  R = refl_coeff_ni (nt, nc);

  for (j = nb - 3; j >= 0; j--)
    {
      cmpl r, rho, beta;
      cmpl den;
      double th = ds[j];

      nc = nt;
      nt = ns[j];

      beta = - 2.0 * I * omega * nc;
      rho = cexp(beta * th);

      r = refl_coeff_ni (nt, nc);

      den = 1 + r * R * rho;
      R = (r + R * rho) / den;
    }

  return R;
}

static cmpl
mult_layer_refl_ni_jacob_th (int nb, const cmpl ns[], const double ds[],
			     double lambda, cmpl *jacth)
{
  const double omega = 2 * M_PI / lambda;
  const int nblyr = nb - 2;
  cmpl R;
  cmpl nt, nc;
  int j;

  nt = ns[nb-2];
  nc = ns[nb-1];

  R = refl_coeff_ni (nt, nc);

  for (j = nb - 3; j >= 0; j--)
    {
      cmpl r, rho, beta, drhodth;
      cmpl dfdR, dfdrho;
      cmpl den, isqden;
      double th = ds[j];
      int k;

      nc = nt;
      nt = ns[j];

      beta = - 2.0 * I * omega * nc;
      rho = cexp(beta * th);
      drhodth = rho * beta;

      r = refl_coeff_ni (nt, nc);

      den = 1 + r * R * rho;
      isqden = 1 / csqr(den);
      dfdR = rho * (1 - r*r) * isqden;

      for (k = nblyr; k > j+1; k--)
	jacth[k-1] *= dfdR;

      dfdrho = R * (1 - r*r) * isqden;

      jacth[j] = dfdrho * drhodth;

      R = (r + R * rho) / den;
    }

  return R;
}

static cmpl
mult_layer_refl_ni_jacob (int nb, const cmpl ns[], const double ds[],
			  double lambda, cmpl *jacth, cmpl *jacn)
{
  const double omega = 2 * M_PI / lambda;
  const int nblyr = nb - 2;
  cmpl R;
  cmpl nt, nc;
  cmpl drdnt, drdnb;
  int j;

  nt = ns[nb-2];
  nc = ns[nb-1];

  R = refl_coeff_ext_ni (nt, nc, &drdnt, &drdnb);

  jacn[nb-1] = drdnb;
  jacn[nb-2] = drdnt;

  for (j = nb - 3; j >= 0; j--)
    {
      cmpl r, rho, beta, drhodn, drhodth;
      cmpl dfdR, dfdr, dfdrho;
      cmpl den, isqden;
      double th = ds[j];
      int k;

      nc = nt;
      nt = ns[j];

      beta = - 2.0 * I * omega * nc;
      rho = cexp(beta * th);
      drhodth = rho * beta;
      drhodn = - 2.0 * I * rho * omega * th;

      r = refl_coeff_ext_ni (nt, nc, &drdnt, &drdnb);

      den = 1 + r * R * rho;
      isqden = 1 / csqr(den);
      dfdR = rho * (1 - r*r) * isqden;

      for (k = nb - 1; k > j+1; k--)
	jacn[k] *= dfdR;

      for (k = nblyr; k > j+1; k--)
	jacth[k-1] *= dfdR;

      dfdr = (1 - csqr(R*rho)) * isqden;
      dfdrho = R * (1 - r*r) * isqden;

      jacn[j+1] = dfdR * jacn[j+1] + dfdr * drdnb + dfdrho * drhodn;
      jacn[j] = dfdr * drdnt;

      jacth[j] = dfdrho * drhodth;

      R = (r + R * rho) / den;
    }

  return R;
}

double
mult_layer_refl_ni (size_t _nb, const cmpl ns[], const double ds[],
		    double lambda,
		    gsl_vector *r_jacob_th, gsl_vector *r_jacob_n)
{
#define NB_JAC_STATIC 10
  struct { cmpl th[NB_JAC_STATIC], n[NB_JAC_STATIC]; } jacs;
  struct { cmpl *th, *n; } jacd;
  int nb = _nb;
  int use_static = (nb <= NB_JAC_STATIC);
  size_t k;
  cmpl r;

  assert (nb > 2);

  if (use_static)
    {
      jacd.th = jacs.th;
      jacd.n  = jacs.n;
    }
  else
    {
      jacd.th = emalloc (nb * sizeof(cmpl));
      jacd.n  = emalloc (nb * sizeof(cmpl));
    }

  if (r_jacob_th && r_jacob_n)
    r = mult_layer_refl_ni_jacob (nb, ns, ds, lambda, jacd.th, jacd.n);
  else if (r_jacob_th)
    r = mult_layer_refl_ni_jacob_th (nb, ns, ds, lambda, jacd.th);
  else
    r = mult_layer_refl_ni_nojacob (nb, ns, ds, lambda);

  if (r_jacob_th)
    for (k = 0; k < (size_t) nb-2; k++)
      {
	cmpl dr = jacd.th[k];
	double drsq = 2 * (creal(r)*creal(dr) + cimag(r)*cimag(dr));
	gsl_vector_set (r_jacob_th, k, drsq);
      }

  if (r_jacob_n)
    for (k = 0; k < (size_t) nb; k++)
      {
	cmpl dr = jacd.n[k];
	double drsqr = 2 * (creal(r)*creal(dr) + cimag(r)*cimag(dr));
	double drsqi = 2 * (cimag(r)*creal(dr) - creal(r)*cimag(dr));
	gsl_vector_set (r_jacob_n, k, drsqr);
	gsl_vector_set (r_jacob_n, nb + k, drsqi);
      }

  if (! use_static)
    {
      free (jacd.th);
      free (jacd.n);
    }

  return CSQABS(r);
#undef NB_JAC_STATIC
}
