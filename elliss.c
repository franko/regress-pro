
/*
  $Id: elliss.c,v 1.12 2006/10/30 22:55:13 francesco Exp $
 */

#include "elliss.h"

static inline cmpl
csqr(cmpl x)
{
  return x*x;
}

static cmpl
refl_coeff (cmpl nt, cmpl cost, cmpl nb, cmpl cosb, polar_t pol)
{
  cmpl rc;

  if (pol == POL_P)
    rc = (nb*cost - nt*cosb) / (nb*cost + nt*cosb);
  else
    rc = (nt*cost - nb*cosb) / (nt*cost + nb*cosb);
  
  return rc;
}

/* NB: drdnt given by this procedure is conceptually wrong if the
   medium of "nt" is the topmost medium (environment). That's because
   we assume that the angle of incidence is fixed a priori. */
static cmpl
refl_coeff_ext (cmpl nt, cmpl cost,
		cmpl nb, cmpl cosb,
		cmpl * drdnt, cmpl * drdnb, polar_t pol)
{
  cmpl rc;

  if (pol == POL_P)
    {
      cmpl den = nb*cost + nt*cosb;
      cmpl isqden = 1 / csqr(den);
      rc = (nb*cost - nt*cosb) / den;
      *drdnt = - 2.0 * cosb * nb * (2*cost*cost-1) * isqden / cost;
      *drdnb =   2.0 * cost * nt * (2*cosb*cosb-1) * isqden / cosb;
    }
  else
    {
      cmpl den = nt*cost + nb*cosb;
      cmpl isqden = 1 / csqr(den);
      rc = (nt*cost - nb*cosb) / den;
      *drdnt =   2.0 * cosb * nb * isqden / cost;
      *drdnb = - 2.0 * cost * nt * isqden / cosb;
    }

  return rc;
}

static cmpl
snell_cos (cmpl nsin0, cmpl nlyr)
{
  cmpl s = nsin0 / nlyr;
  return csqrt(1.0 - csqr(s));
}

static void
mult_layer_refl (int nb, const cmpl ns[], cmpl nsin0, 
		 const double ds[], double lambda, cmpl R[])
{
  const double omega = 2 * M_PI / lambda;
  cmpl cosc, cost;
  const cmpl *nptr;
  int j;

  /* In this procedure we assume (nb > 2). This condition should be
     ensured in advance. */

  nptr = ns + (nb-2);

  cost = snell_cos (nsin0, nptr[0]);
  cosc = snell_cos (nsin0, nptr[1]);

  R[0] = refl_coeff (nptr[0], cost, nptr[1], cosc, POL_S);
  R[1] = refl_coeff (nptr[0], cost, nptr[1], cosc, POL_P);

  for (j = nb - 3; j >= 0; j--)
    {
      cmpl r[2], rho, beta;
      double th = ds[j];
      polar_t p;

      nptr --;

      cosc = cost;
      cost = snell_cos (nsin0, nptr[0]);

      beta = - 2.0 * I * omega * nptr[1] * cosc;
      rho = cexp(beta * th);

      for (p = 0; p <= 1; p++)
	{
	  r[p] = refl_coeff (nptr[0], cost, nptr[1], cosc, p);

	  R[p] = (r[p] + R[p] * rho) / (1 + r[p] * R[p] * rho);
      }
    }
}

static void
mult_layer_refl_jacob_th (int nb, const cmpl ns[], cmpl nsin0, 
			  const double ds[], double lambda, cmpl R[],
			  cmpl *jacth)
{
  const double omega = 2 * M_PI / lambda;
  const int nblyr = nb - 2;
  cmpl cosc, cost;
  const cmpl *nptr;
  int j;

  /* In this procedure we assume (nb > 2). This condition should be
     ensured in advance. */

  nptr = ns + (nb-2);

  cost = snell_cos (nsin0, nptr[0]);
  cosc = snell_cos (nsin0, nptr[1]);

  R[0] = refl_coeff (nptr[0], cost, nptr[1], cosc, POL_S);
  R[1] = refl_coeff (nptr[0], cost, nptr[1], cosc, POL_P);

  for (j = nb - 3; j >= 0; j--)
    {
      cmpl r[2], rho, beta, drhodth;
      double th = ds[j];
      polar_t p;
      int k;

      nptr --;

      cosc = cost;
      cost = snell_cos (nsin0, nptr[0]);

      beta = - 2.0 * I * omega * nptr[1] * cosc;
      rho = cexp(beta * th);
      drhodth = rho * beta;

      for (p = 0; p <= 1; p++)
	{
	  cmpl dfdR, dfdrho;
	  cmpl *pjacth = jacth + (p == 0 ? 0 : nblyr);
	  cmpl den, isqden;

	  r[p] = refl_coeff (nptr[0], cost, nptr[1], cosc, p);

	  den = 1 + r[p] * R[p] * rho;
	  isqden = 1 / csqr(den);
	  dfdR = rho * (1 - r[p]*r[p]) * isqden;

	  for (k = nblyr; k > j+1; k--)
	    pjacth[k-1] *= dfdR;

	  dfdrho = R[p] * (1 - r[p]*r[p]) * isqden;

	  pjacth[j] = dfdrho * drhodth;

	  R[p] = (r[p] + R[p] * rho) / den;
      }
    }
}

static void
mult_layer_refl_jacob (int nb, const cmpl ns[], cmpl nsin0, 
		       const double ds[], double lambda, cmpl R[],
		       cmpl *jacth, cmpl *jacn)
{
  const double omega = 2 * M_PI / lambda;
  const int nblyr = nb - 2;
  cmpl cosc, cost;
  const cmpl *nptr;
  cmpl drdnt[2], drdnb[2];
  int j;

  /* In this procedure we assume (nb > 2). This condition should be
     ensured in advance. */

  nptr = ns + (nb-2);

  cost = snell_cos (nsin0, nptr[0]);
  cosc = snell_cos (nsin0, nptr[1]);

  R[0] = refl_coeff_ext (nptr[0], cost, nptr[1], cosc, 
			 &drdnt[0], &drdnb[0], POL_S);
  R[1] = refl_coeff_ext (nptr[0], cost, nptr[1], cosc,
			 &drdnt[1], &drdnb[1], POL_P);

  jacn[nb-1]      = drdnb[0];
  jacn[nb + nb-1] = drdnb[1];

  jacn[nb-2]      = drdnt[0];
  jacn[nb + nb-2] = drdnt[1];

  for (j = nb - 3; j >= 0; j--)
    {
      cmpl r[2], rho, beta, drhodn, drhodth;
      double th = ds[j];
      polar_t p;
      int k;

      nptr --;

      cosc = cost;
      cost = snell_cos (nsin0, nptr[0]);

      beta = - 2.0 * I * omega * nptr[1] * cosc;
      rho = cexp(beta * th);
      drhodth = rho * beta;
      drhodn = - 2.0 * I * rho * omega * th / cosc;

      for (p = 0; p <= 1; p++)
	{
	  cmpl dfdR, dfdr, dfdrho;
	  cmpl *pjacn  = jacn  + (p == 0 ? 0 : nb);
	  cmpl *pjacth = jacth + (p == 0 ? 0 : nblyr);
	  cmpl den, isqden;

	  r[p] = refl_coeff_ext (nptr[0], cost, nptr[1], cosc,
				 &drdnt[p], &drdnb[p], p);

	  den = 1 + r[p] * R[p] * rho;
	  isqden = 1 / csqr(den);
	  dfdR = rho * (1 - r[p]*r[p]) * isqden;

	  for (k = nb - 1; k > j+1; k--)
	    pjacn[k] *= dfdR;

	  for (k = nblyr; k > j+1; k--)
	    pjacth[k-1] *= dfdR;

	  dfdr = (1 - csqr(R[p]*rho)) * isqden;
	  dfdrho = R[p] * (1 - r[p]*r[p]) * isqden;

	  pjacn[j+1] = dfdR * pjacn[j+1] + dfdr * drdnb[p] + dfdrho * drhodn;
	  pjacn[j] = (j == 0 ? 0.0 : dfdr * drdnt[p]);

	  pjacth[j] = dfdrho * drhodth;

	  R[p] = (r[p] + R[p] * rho) / den;
      }
    }
}

#if 0
static void
multlayer_refl_na (int nb, const cmpl ns[], cmpl nsin0, 
		   const double ds[], double lambda, cmpl R[],
		   double numap)
{
  const int ndiv = 16;
  double phi0 = asin (creal(nsin0 / ns[0])); // should be real
  double dphi = asin (numap);
  double xr[3];
  int j;

  R[0] = R[1] = 0.0i;

  xr[2] = -1.0;
  for (j = -ndiv/2 + 1; j <= ndiv/2; j++)
    {
      double dxr;
      int k, klim;

      xr[0] = xr[2];
      xr[2] = sin(2 * j * M_PI_2 / ndiv);
      xr[1] = (xr[0] + xr[2]) / 2;

      dxr = xr[2] - xr[0];

      klim = (j == ndiv/2 ? 2 : 3);

      for (k = 1; k < klim; k++)
	{
	  const int sc[3] = {1, 4, 2};
	  cmpl nsin0x = ns[0] * sin(phi0 + dphi * xr[k]);
	  double rf = sqrt(1 - xr[k] * xr[k]);
	  cmpl Rx[2];
      
	  mult_layer_refl (nb, ns, nsin0x, ds, lambda, Rx);

	  R[0] += rf * sc[k] * dxr * Rx[0] / (6 * M_PI_2);
	  R[1] += rf * sc[k] * dxr * Rx[1] / (6 * M_PI_2);
	}
    }
}
#endif

/* NB: In this case we are treating a Psi-Delta spectrum and
   the fields named alpha and beta corresponds actually to
   tan(psi) and cos(delta), respectively. */
static void
se_psidel (cmpl R[], ell_ab_t e)
{
  cmpl rho = R[1] / R[0];
  e->alpha = cabs(rho);
  e->beta  = creal(rho) / e->alpha;
}

static void
se_psidel_der (cmpl R[], cmpl dR[], cmpl *dtpsi, cmpl *dcdelta)
{
  cmpl rho = R[1] / R[0];
  cmpl drho = (R[0]*dR[1] - R[1]*dR[0]) / (R[0]*R[0]);
  double irhosq = 1 / CSQABS(rho);
  double iden = sqrt(irhosq);

  *dtpsi = iden * conj(rho) * drho;
  *dcdelta = iden * (1 - conj(rho) / rho) * drho / 2.0;
}

static void
se_ab (cmpl R[], double tanlz, ell_ab_t e)
{
  cmpl rho = R[1] / R[0];
  double sqtpsi = CSQABS(rho);
  double tasq = tanlz * tanlz;
  double iden = 1 / (sqtpsi + tasq);
  e->alpha = (sqtpsi - tasq) * iden;
  e->beta = 2 * creal(rho) * tanlz * iden;
}

static void
se_ab_der (cmpl R[], cmpl dR[], double tanlz, cmpl *dalpha, cmpl *dbeta)
{
  cmpl rho = R[1] / R[0];
  cmpl drho = (R[0]*dR[1] - R[1]*dR[0]) / (R[0]*R[0]);
  double sqtpsi = CSQABS(rho);
  double tasq = tanlz * tanlz;
  double isqden;
  cmpl z;

  isqden = 1 / (sqtpsi + tasq);
  isqden *= isqden;

  z = conj(rho) * drho;
  *dalpha = 4 * tasq * z * isqden;

  z = conj(tasq - rho*rho) * drho;
  *dbeta = 2 * tanlz * z * isqden;
}

void
mult_layer_se_jacob (enum se_type type,
		     size_t _nb, const cmpl ns[], double phi0, 
		     const double ds[], double lambda,
		     double anlz, ell_ab_t e,
		     gsl_vector *jacob_th, cmpl_vector *jacob_n)
{
#define NB_JAC_STATIC 10
  static struct { cmpl th[2*NB_JAC_STATIC], n[2*NB_JAC_STATIC]; } jacs;
  struct { cmpl *th, *n; } jac;
  const int nb = _nb, nblyr = nb - 2;
  int use_static = (nb <= NB_JAC_STATIC);
  double tanlz = tan(anlz);
  cmpl R[2], nsin0;
  size_t j;

  if (use_static)
    {
      jac.th = jacs.th;
      jac.n  = jacs.n;
    }
  else
    {
      jac.th = emalloc (2*nb*sizeof(cmpl));
      jac.n  = emalloc (2*nb*sizeof(cmpl));
    }

  nsin0 = ns[0] * csin ((cmpl) phi0);

  //  multlayer_refl_na (nb, ns, nsin0, ds, lambda, R, 0.29);

  if (jacob_th && jacob_n)
    mult_layer_refl_jacob (nb, ns, nsin0, ds, lambda, R, jac.th, jac.n);
  else if (jacob_th)
    mult_layer_refl_jacob_th (nb, ns, nsin0, ds, lambda, R, jac.th);
  else
    mult_layer_refl (nb, ns, nsin0, ds, lambda, R);

  if (type == SE_ALPHA_BETA)
    se_ab (R, tanlz, e);
  else
    se_psidel (R, e);

  if (jacob_n)
    {
      /* we set the derivative respect to the RI of the ambient to 0.0 */
      cmpl_vector_set (jacob_n, 0, 0.0i);
      cmpl_vector_set (jacob_n, (size_t) nb, 0.0i);
      for (j = 1; j < (size_t) nb; j++)
	{
	  struct { cmpl alpha, beta; } d;
	  cmpl dR[2] = {jac.n[j], jac.n[nb+j]};

	  if (type == SE_ALPHA_BETA)
	    se_ab_der (R, dR, tanlz, &d.alpha, &d.beta);
	  else
	    se_psidel_der (R, dR, &d.alpha, &d.beta);

	  cmpl_vector_set (jacob_n, j,    d.alpha);
	  cmpl_vector_set (jacob_n, nb+j, d.beta);
	}
    }

  if (jacob_th)
    {
      for (j = 0; j < (size_t) nblyr; j++)
	{
	  struct { cmpl alpha, beta; } d;
	  cmpl dR[2] = {jac.th[j], jac.th[nblyr+j]};

	  if (type == SE_ALPHA_BETA)
	    se_ab_der (R, dR, tanlz, &d.alpha, &d.beta);
	  else
	    se_psidel_der (R, dR, &d.alpha, &d.beta);

	  gsl_vector_set (jacob_th, j, creal(d.alpha));
	  gsl_vector_set (jacob_th, nblyr+j, creal(d.beta));
	}
    }

  if (! use_static)
    {
      free (jac.th);
      free (jac.n);
    }
}
