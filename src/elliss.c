
/* elliss.c
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
#include "elliss.h"

static double deg_to_radians(double x) { return x * M_PI / 180.0; }

static inline cmpl
csqr(cmpl x)
{
    return x*x;
}

static cmpl
refl_coeff(cmpl nt, cmpl cost, cmpl nb, cmpl cosb, polar_t pol)
{
    cmpl rc;

    if(pol == POL_P) {
        rc = (nb*cost - nt*cosb) / (nb*cost + nt*cosb);
    } else {
        rc = (nt*cost - nb*cosb) / (nt*cost + nb*cosb);
    }

    return rc;
}

/* Compute Fresnel reflection coefficient and their derivative wrt the
   angle of incidence (AOI).
   The parameter nsin0 and ncos0 refer to the measurement environment
   (the air usually). */
static cmpl
refl_coeff_aoi_der(const cmpl nsin0, const cmpl ncos0, cmpl n0, cmpl cos0, cmpl n1, cmpl cos1, cmpl *drdaoi, polar_t pol)
{
    const cmpl drdaoi_prefact = 2 * nsin0 * ncos0 * ((n0*cos0) / (n1*cos1) - (n1*cos1) / (n0*cos0));
    if(pol == POL_P) {
        const cmpl den = n1*cos0 + n0*cos1;
        *drdaoi = drdaoi_prefact / csqr(den);
        return (n1*cos0 - n0*cos1) / den;
    }
    /* S polarization. */
    const cmpl den = n0*cos0 + n1*cos1;
    *drdaoi = drdaoi_prefact / csqr(den);
    return (n0*cos0 - n1*cos1) / den;
}

/* NB: drdnt given by this procedure is conceptually wrong if the
   medium of "nt" is the topmost medium (environment). That's because
   we assume that the angle of incidence is fixed a priori. */
static cmpl
refl_coeff_ext(cmpl nt, cmpl cost,
               cmpl nb, cmpl cosb,
               cmpl * drdnt, cmpl * drdnb, cmpl *den_out, polar_t pol)
{
    cmpl rc;

    if(pol == POL_P) {
        cmpl den = nb*cost + nt*cosb;
        cmpl isqden = 1 / csqr(den);
        rc = (nb*cost - nt*cosb) / den;
        *drdnt = - 2.0 * cosb * nb * (2*cost*cost-1) * isqden / cost;
        *drdnb =   2.0 * cost * nt * (2*cosb*cosb-1) * isqden / cosb;
        *den_out = den;
    } else {
        cmpl den = nt*cost + nb*cosb;
        cmpl isqden = 1 / csqr(den);
        rc = (nt*cost - nb*cosb) / den;
        *drdnt =   2.0 * cosb * nb * isqden / cost;
        *drdnb = - 2.0 * cost * nt * isqden / cosb;
        *den_out = den;
    }

    return rc;
}

static void
fresnel_coeffs_diff(const cmpl nsin0, const cmpl ncos0, const cmpl n0, const cmpl cos0, const cmpl n1, const cmpl cos1, cmpl r[2], cmpl drdn0[2], cmpl drdn1[2], cmpl drdaoi[2])
{
    const cmpl drdaoi_prefact = 2 * nsin0 * ncos0 * ((n0 * cos0) / (n1 * cos1) - (n1 * cos1) / (n0 * cos0));
    for(polar_t p = 0; p <= 1; p++) {
        cmpl fc_den;
        r[p] = refl_coeff_ext(n0, cos0, n1, cos1, &drdn0[p], &drdn1[p], &fc_den, p);
        drdaoi[p] = drdaoi_prefact / csqr(fc_den);
    }
}

static cmpl
snell_cos(cmpl nsin0, cmpl nlyr)
{
    cmpl s = nsin0 / nlyr;
    return csqrt(1.0 - csqr(s));
}

/* In this procedure we assume (nb > 2). This condition should be
   ensured in advance. */
static void
mult_layer_refl(int nb, const cmpl ns[], cmpl nsin0,
                const double ds[], double lambda, cmpl R[])
{
    const double omega = 2 * M_PI / lambda;
    int j = nb - 2; /* Start from the layer just above the substrate. */

    cmpl cost = snell_cos(nsin0, ns[j]);
    cmpl cosc = snell_cos(nsin0, ns[j+1]);

    R[0] = refl_coeff(ns[j], cost, ns[j+1], cosc, POL_S);
    R[1] = refl_coeff(ns[j], cost, ns[j+1], cosc, POL_P);

    for(j--; j >= 0; j--) { /* Iterate through layers from bottom to top. */
        cosc = cost;
        cost = snell_cos(nsin0, ns[j]);

        const cmpl beta = - 2.0 * I * omega * ns[j+1] * cosc;
        const cmpl rho = cexp(beta * THICKNESS_TO_NM(ds[j]));

        cmpl r[2];
        for(polar_t p = 0; p <= 1; p++) {
            r[p] = refl_coeff(ns[j], cost, ns[j+1], cosc, p);

            R[p] = (r[p] + R[p] * rho) / (1 + r[p] * R[p] * rho);
        }
    }
}

static void
mult_layer_refl_jacob_th(int nb, const cmpl ns[], cmpl nsin0,
                         const double ds[], double lambda, cmpl R[],
                         cmpl *jacth)
{
    const double omega = 2 * M_PI / lambda;
    const int films_number = nb - 2;
    int j = nb - 2; /* Start from the layer just above the substrate. */

    cmpl cost = snell_cos(nsin0, ns[j]);
    cmpl cosc = snell_cos(nsin0, ns[j+1]);

    R[0] = refl_coeff(ns[j], cost, ns[j+1], cosc, POL_S);
    R[1] = refl_coeff(ns[j], cost, ns[j+1], cosc, POL_P);

    for(j--; j >= 0; j--) { /* Iterate through layers from bottom to top. */
        cosc = cost;
        cost = snell_cos(nsin0, ns[j]);

        const cmpl beta = - 2.0 * I * omega * ns[j+1] * cosc;
        const cmpl rho = cexp(beta * THICKNESS_TO_NM(ds[j]));
        const cmpl drhodth = rho * beta * THICKNESS_TO_NM(1.0);

        cmpl r[2];
        for(polar_t p = 0; p <= 1; p++) {
            cmpl *pjacth = jacth + (p == 0 ? 0 : films_number);

            r[p] = refl_coeff(ns[j], cost, ns[j+1], cosc, p);

            const cmpl den = 1 + r[p] * R[p] * rho;
            const cmpl isqden = 1 / csqr(den);
            const cmpl dfdR = rho * (1 - r[p]*r[p]) * isqden;

            for(int k = films_number; k > j+1; k--) {
                pjacth[k - 1] *= dfdR;
            }

            const cmpl dfdrho = R[p] * (1 - r[p]*r[p]) * isqden;
            pjacth[j] = dfdrho * drhodth;

            R[p] = (r[p] + R[p] * rho) / den;
        }
    }
}

static void
mult_layer_refl_jacob_th_aoi(int nb, const cmpl ns[], cmpl nsin0,
                            const double ds[], double lambda, cmpl R[2],
                            cmpl dRdaoi[2], cmpl *jacth)
{
    const double omega = 2 * M_PI / lambda;
    const cmpl bphase = 2.0 * I * omega;
    const int films_number = nb - 2;
    int j = nb - 2; /* Start from the layer just above the substrate. */

    const cmpl ncos0 = snell_cos(nsin0, ns[0]) * ns[0];

    cmpl cost = snell_cos(nsin0, ns[j]);
    cmpl cosc = snell_cos(nsin0, ns[j+1]);

    R[0] = refl_coeff_aoi_der(nsin0, ncos0, ns[j], cost, ns[j+1], cosc, &dRdaoi[0], POL_S);
    R[1] = refl_coeff_aoi_der(nsin0, ncos0, ns[j], cost, ns[j+1], cosc, &dRdaoi[1], POL_P);

    for(j--; j >= 0; j--) { /* Iterate through layers from bottom to top. */
        cosc = cost;
        cost = snell_cos(nsin0, ns[j]);

        const cmpl beta = - bphase * ns[j+1] * cosc;
        const cmpl rho = cexp(beta * THICKNESS_TO_NM(ds[j]));
        const cmpl drhodth = rho * beta * THICKNESS_TO_NM(1.0);
        const cmpl drhodaoi = rho * bphase * nsin0 * ncos0 / (ns[j+1] * cosc) * THICKNESS_TO_NM(ds[j]);

        cmpl r[2], drdaoi[2];
        for(polar_t p = 0; p <= 1; p++) {
            cmpl *pjacth = jacth + (p == 0 ? 0 : films_number);

            r[p] = refl_coeff_aoi_der(nsin0, ncos0, ns[j], cost, ns[j+1], cosc, &drdaoi[p], p);

            const cmpl den = 1 + r[p] * R[p] * rho;
            const cmpl isqden = 1 / csqr(den);
            const cmpl dfdR = rho * (1 - r[p]*r[p]) * isqden;

            for(int k = films_number; k > j+1; k--) {
                pjacth[k - 1] *= dfdR;
            }

            const cmpl dfdr = (1 - csqr(R[p]*rho)) * isqden;
            const cmpl dfdrho = R[p] * (1 - r[p]*r[p]) * isqden;
            pjacth[j] = dfdrho * drhodth;

            dRdaoi[p] = dfdr * drdaoi[p] + dfdrho * drhodaoi + dfdR * dRdaoi[p];

            R[p] = (r[p] + R[p] * rho) / den;
        }
    }
}

static void
mult_layer_refl_jacob(int nb, const cmpl ns[], cmpl nsin0,
                      const double ds[], double lambda,
                      cmpl R[2], cmpl dRdaoi[2], cmpl *jacth, cmpl *jacn)
{
    const double omega = 2 * M_PI / lambda;
    const cmpl bphase = 2.0 * I * omega;
    const int films_number = nb - 2;
    cmpl drdnt[2], drdnb[2];
    cmpl drdaoi[2];
    int j = nb - 2; /* Start from the layer just above the substrate. */

    const cmpl ncos0 = snell_cos(nsin0, ns[0]) * ns[0];

    cmpl cost = snell_cos(nsin0, ns[j]);
    cmpl cosc = snell_cos(nsin0, ns[j+1]);

    fresnel_coeffs_diff(nsin0, ncos0, ns[j], cost, ns[j+1], cosc, R, drdnt, drdnb, dRdaoi);

    jacn[nb-1]      = drdnb[0];
    jacn[nb + nb-1] = drdnb[1];

    jacn[nb-2]      = drdnt[0];
    jacn[nb + nb-2] = drdnt[1];

    for(j--; j >= 0; j--) { /* Iterate through layers from bottom to top. */
        const double th = ds[j];

        cosc = cost;
        cost = snell_cos(nsin0, ns[j]);

        const cmpl beta = - bphase * ns[j+1] * cosc;
        const cmpl rho = cexp(beta * THICKNESS_TO_NM(th));
        const cmpl drhodth = rho * beta * THICKNESS_TO_NM(1.0);
        const cmpl drhodn = - bphase * rho * THICKNESS_TO_NM(th) / cosc;
        const cmpl drhodaoi = rho * bphase * nsin0 * ncos0 / (ns[j+1] * cosc) * THICKNESS_TO_NM(th);

        cmpl r[2];
        fresnel_coeffs_diff(nsin0, ncos0, ns[j], cost, ns[j+1], cosc, r, drdnt, drdnb, drdaoi);

        for(polar_t p = 0; p <= 1; p++) {
            cmpl *pjacn  = jacn  + (p == 0 ? 0 : nb);
            cmpl *pjacth = jacth + (p == 0 ? 0 : films_number);

            const cmpl den = 1 + r[p] * R[p] * rho;
            const cmpl isqden = 1 / csqr(den);
            const cmpl dfdR = rho * (1 - r[p]*r[p]) * isqden;

            for(int k = nb - 1; k > j+1; k--) {
                pjacn[k] *= dfdR;
            }

            for(int k = films_number; k > j+1; k--) {
                pjacth[k-1] *= dfdR;
            }

            const cmpl dfdr = (1 - csqr(R[p]*rho)) * isqden;
            const cmpl dfdrho = R[p] * (1 - r[p]*r[p]) * isqden;

            pjacn[j+1] = dfdR * pjacn[j+1] + dfdr * drdnb[p] + dfdrho * drhodn;
            pjacn[j] = (j == 0 ? 0.0 : dfdr * drdnt[p]);

            pjacth[j] = dfdrho * drhodth;

            dRdaoi[p] = dfdr * drdaoi[p] + dfdrho * drhodaoi + dfdR * dRdaoi[p];

            R[p] = (r[p] + R[p] * rho) / den;
        }
    }
}

#if 0
static void
multlayer_refl_na(int nb, const cmpl ns[], cmpl nsin0,
                  const double ds[], double lambda, cmpl R[],
                  double numap)
{
    const int ndiv = 16;
    double phi0 = asin(creal(nsin0 / ns[0]));  // should be real
    double dphi = asin(numap);
    double xr[3];
    int j;

    R[0] = R[1] = 0.0i;

    xr[2] = -1.0;
    for(j = -ndiv/2 + 1; j <= ndiv/2; j++) {
        double dxr;
        int k, klim;

        xr[0] = xr[2];
        xr[2] = sin(2 * j * M_PI_2 / ndiv);
        xr[1] = (xr[0] + xr[2]) / 2;

        dxr = xr[2] - xr[0];

        klim = (j == ndiv/2 ? 2 : 3);

        for(k = 1; k < klim; k++) {
            const int sc[3] = {1, 4, 2};
            cmpl nsin0x = ns[0] * sin(phi0 + dphi * xr[k]);
            double rf = sqrt(1 - xr[k] * xr[k]);
            cmpl Rx[2];

            mult_layer_refl(nb, ns, nsin0x, ds, lambda, Rx);

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
se_psidel(cmpl R[], ell_ab_t e)
{
    cmpl rho = R[1] / R[0];
    e->alpha = cabs(rho);
    e->beta  = creal(rho) / e->alpha;
}

static void
se_psidel_der(cmpl R[], cmpl dR[], cmpl *dtpsi, cmpl *dcdelta)
{
    cmpl rho = R[1] / R[0];
    cmpl drho = (R[0]*dR[1] - R[1]*dR[0]) / (R[0]*R[0]);
    double irhosq = 1 / CSQABS(rho);
    double iden = sqrt(irhosq);

    *dtpsi = iden * conj(rho) * drho;
    *dcdelta = iden * (1 - conj(rho) / rho) * drho / 2.0;
}

static void
se_ab(cmpl R[], double tanlz, ell_ab_t e)
{
    cmpl rho = R[1] / R[0];
    double sqtpsi = CSQABS(rho);
    double tasq = tanlz * tanlz;
    double iden = 1 / (sqtpsi + tasq);
    e->alpha = (sqtpsi - tasq) * iden;
    e->beta = 2 * creal(rho) * tanlz * iden;
}

static void
se_ab_der(cmpl R[], cmpl dR[], double tanlz, cmpl *dalpha, cmpl *dbeta)
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
mult_layer_se_jacob(enum se_type type,
                    size_t _nb, const cmpl ns[], double phi0,
                    const double ds[], double lambda,
                    double anlz, ell_ab_t e,
                    double *aoi_der, gsl_vector *jacob_th, cmpl_vector *jacob_n)
{
    const int nb = _nb, nblyr = nb - 2;
    cmpl mlr_jacob_th[2 * nb], mlr_jacob_n[2 * nb];
    double tanlz = tan(anlz);
    cmpl R[2], dRdaoi[2], nsin0;
    size_t j;

    nsin0 = ns[0] * csin((cmpl) phi0);

    if(jacob_th && jacob_n) {
        mult_layer_refl_jacob(nb, ns, nsin0, ds, lambda, R, dRdaoi, mlr_jacob_th, mlr_jacob_n);
    } else if (jacob_th || aoi_der) {
        mult_layer_refl_jacob_th_aoi(nb, ns, nsin0, ds, lambda, R, dRdaoi, mlr_jacob_th);
    } else if (jacob_th) {
        mult_layer_refl_jacob_th(nb, ns, nsin0, ds, lambda, R, mlr_jacob_th);
    } else {
        mult_layer_refl(nb, ns, nsin0, ds, lambda, R);
    }

    if(type == SE_ALPHA_BETA) {
        se_ab(R, tanlz, e);
    } else {
        se_psidel(R, e);
    }

    if(jacob_n) {
        /* we set the derivative respect to the RI of the ambient to 0.0 */
        cmpl_vector_set(jacob_n, 0, 0.0i);
        cmpl_vector_set(jacob_n, (size_t) nb, 0.0i);
        for(j = 1; j < (size_t) nb; j++) {
            cmpl d_alpha, d_beta;
            cmpl dR[2] = {mlr_jacob_n[j], mlr_jacob_n[nb+j]};

            if(type == SE_ALPHA_BETA) {
                se_ab_der(R, dR, tanlz, &d_alpha, &d_beta);
            } else {
                se_psidel_der(R, dR, &d_alpha, &d_beta);
            }

            cmpl_vector_set(jacob_n, j,    d_alpha);
            cmpl_vector_set(jacob_n, nb+j, d_beta);
        }
    }

    if(jacob_th) {
        for(j = 0; j < (size_t) nblyr; j++) {
            cmpl d_alpha, d_beta;
            cmpl dR[2] = {mlr_jacob_th[j], mlr_jacob_th[nblyr+j]};

            if(type == SE_ALPHA_BETA) {
                se_ab_der(R, dR, tanlz, &d_alpha, &d_beta);
            } else {
                se_psidel_der(R, dR, &d_alpha, &d_beta);
            }

            gsl_vector_set(jacob_th, j, creal(d_alpha));
            gsl_vector_set(jacob_th, nblyr+j, creal(d_beta));
        }
    }

    if (aoi_der) {
        cmpl d_alpha, d_beta;
        if(type == SE_ALPHA_BETA) {
            se_ab_der(R, dRdaoi, tanlz, &d_alpha, &d_beta);
        } else {
            se_psidel_der(R, dRdaoi, &d_alpha, &d_beta);
        }
        aoi_der[0] = deg_to_radians(creal(d_alpha));
        aoi_der[1] = deg_to_radians(creal(d_beta));
    }
}
