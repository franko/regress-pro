
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

static double sqr(double x) { return x*x; }

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
se_psidel_der_acquisition(cmpl R[], cmpl dR[], double *jac)
{
    const cmpl rho = R[1] / R[0];
    const cmpl drho = (R[0]*dR[1] - R[1]*dR[0]) / (R[0]*R[0]);
    const double irhosq = 1 / CSQABS(rho);
    const double iden = sqrt(irhosq);

    /* Derivatives with AOI. */
    jac[SE_ACQ_INDEX(SE_TANPSI, SE_AOI)] = creal(iden * conj(rho) * drho);
    jac[SE_ACQ_INDEX(SE_COSDEL, SE_AOI)] = creal(iden * (1 - conj(rho) / rho) * drho / 2.0);
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
    const cmpl rho = R[1] / R[0];
    const cmpl drho = (R[0]*dR[1] - R[1]*dR[0]) / (R[0]*R[0]);
    const double sqtpsi = CSQABS(rho);
    const double tasq = tanlz * tanlz;
    const double isqden = sqr(1 / (sqtpsi + tasq));

    const cmpl z1 = conj(rho) * drho;
    *dalpha = 4 * tasq * z1 * isqden;

    const cmpl z2 = conj(tasq - rho*rho) * drho;
    *dbeta = 2 * tanlz * z2 * isqden;
}


static void
se_ab_der_acquisition(cmpl R[], cmpl dR[], double tanlz, double *jac)
{
    const cmpl rho = R[1] / R[0];
    const cmpl drho = (R[0]*dR[1] - R[1]*dR[0]) / (R[0]*R[0]);
    const double sqtpsi = CSQABS(rho);
    const double tasq = tanlz * tanlz;

    double isqden = 1 / (sqtpsi + tasq);
    isqden *= isqden;

    const cmpl z1 = conj(rho) * drho;
    const cmpl z2 = conj(tasq - rho*rho) * drho;

    /* Derivatives with AOI. */
    jac[SE_ACQ_INDEX(SE_ALPHA, SE_AOI)] = creal(4 * tasq * z1 * isqden);
    jac[SE_ACQ_INDEX(SE_BETA , SE_AOI)] = creal(2 * tanlz * z2 * isqden);

    /* Derivatives with Analyzer angle (A). */
    const double secsqa = 1 + tasq; /* = 1 / cos^2(A) = sec^2(A) */
    jac[SE_ACQ_INDEX(SE_ALPHA, SE_ANALYZER)] = - 4 * tanlz * sqtpsi * isqden * secsqa;
    jac[SE_ACQ_INDEX(SE_BETA , SE_ANALYZER)] = 2 * creal(rho) * (sqtpsi - tasq) * isqden * secsqa;
}

static void
set_se_measuring_jacobian_real(enum se_type se_type, const int n, cmpl R[2], const double tanlz, const cmpl *mlr_jacob, gsl_vector *jacob)
{
    for(int j = 0; j < n; j++) {
        cmpl d_alpha, d_beta;
        cmpl dR[2] = {mlr_jacob[j], mlr_jacob[n + j]};
        if(se_type == SE_ALPHA_BETA) {
            se_ab_der(R, dR, tanlz, &d_alpha, &d_beta);
        } else {
            se_psidel_der(R, dR, &d_alpha, &d_beta);
        }
        gsl_vector_set(jacob, j    , creal(d_alpha));
        gsl_vector_set(jacob, n + j, creal(d_beta));
    }
}

static void
set_se_measuring_jacobian_complex(enum se_type se_type, const int n, cmpl R[2], const double tanlz, const cmpl *mlr_jacob, cmpl_vector *jacob)
{
    for(int j = 0; j < n; j++) {
        cmpl d_alpha, d_beta;
        cmpl dR[2] = {mlr_jacob[j], mlr_jacob[n + j]};
        if(se_type == SE_ALPHA_BETA) {
            se_ab_der(R, dR, tanlz, &d_alpha, &d_beta);
        } else {
            se_psidel_der(R, dR, &d_alpha, &d_beta);
        }
        cmpl_vector_set(jacob, j    , d_alpha);
        cmpl_vector_set(jacob, n + j, d_beta);
    }

}

void
mult_layer_se_jacob(enum se_type type,
                    size_t _nb, const cmpl ns[], double phi0,
                    const double ds[], double lambda,
                    double anlz, ell_ab_t e,
                    gsl_vector *jacob_th, cmpl_vector *jacob_n, double *jacob_acquisition)
{
    const int nb = _nb, nblyr = nb - 2;
    cmpl mlr_jacob_th[2 * nb], mlr_jacob_n[2 * nb];
    double tanlz = tan(anlz);
    cmpl R[2], dRdaoi[2], nsin0;

    nsin0 = ns[0] * csin((cmpl) phi0);

    if(jacob_th && jacob_n) {
        mult_layer_refl_jacob(nb, ns, nsin0, ds, lambda, R, dRdaoi, mlr_jacob_th, mlr_jacob_n);
    } else if (jacob_th || jacob_acquisition) {
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
        mlr_jacob_n[0 ] = 0.0;
        mlr_jacob_n[nb] = 0.0;

        set_se_measuring_jacobian_complex(type, nb, R, tanlz, mlr_jacob_n, jacob_n);
    }

    if(jacob_th) {
        set_se_measuring_jacobian_real(type, nblyr, R, tanlz, mlr_jacob_th, jacob_th);
    }

    if (jacob_acquisition) {
        if(type == SE_ALPHA_BETA) {
            se_ab_der_acquisition(R, dRdaoi, tanlz, jacob_acquisition);
        } else {
            se_psidel_der_acquisition(R, dRdaoi, jacob_acquisition);
        }
        for (int i = 0; i < 2 * SE_ACQ_PARAMETERS_NB(type); i++) {
            jacob_acquisition[i] = deg_to_radians(jacob_acquisition[i]);
        }
    }
}

static void
sp_products(const cmpl R[2], double tanlz, double sp[3])
{
    sp[0] = CSQABS(R[0]); /* |Rs|^2 */
    sp[1] = CSQABS(R[1]); /* |Rp|^2 */
    sp[2] = creal(R[0] * conj(R[1])); /* 1/2 (Rs Rp* + Rs* Rp) = Re(Rs Rp*) */
}

static void
sp_products_der(const cmpl R[2], const cmpl dR[2], double tanlz, cmpl dsp[3])
{
    dsp[0] = 2 * R[0] * dR[0]; /* derivative of |Rs|^2 */
    dsp[1] = 2 * R[1] * dR[1]; /* derivative of |Rp|^2 */
    dsp[2] = R[0] * conj(dR[1]) + R[1] * conj(dR[0]); /* derivative of Re(Rs Rp*) */
}

void
mult_layer_sp_products_jacob(const int nb, const cmpl nsin0, const cmpl ns[],
                    const double ds[], const double lambda,
                    const double tanlz, double sp[3],
                    double *jacob_th, cmpl *jacob_n, double *dsp_daoi)
{
    const int nblyr = nb - 2;
    cmpl mlr_jacob_th[2 * nb], mlr_jacob_n[2 * nb];
    cmpl R[2], dRdaoi[2];

    if(jacob_th && jacob_n) {
        mult_layer_refl_jacob(nb, ns, nsin0, ds, lambda, R, dRdaoi, mlr_jacob_th, mlr_jacob_n);
    } else if (jacob_th || dsp_daoi) {
        mult_layer_refl_jacob_th_aoi(nb, ns, nsin0, ds, lambda, R, dRdaoi, mlr_jacob_th);
    } else if (jacob_th) {
        mult_layer_refl_jacob_th(nb, ns, nsin0, ds, lambda, R, mlr_jacob_th);
    } else {
        mult_layer_refl(nb, ns, nsin0, ds, lambda, R);
    }

    sp_products(R, tanlz, sp);

    if(jacob_n) {
        for(int j = 0; j < nb; j++) {
            if (j == 0) {
                /* we set the derivative respect to the RI of the ambient to 0.0 */
                for (int k = 0; k < 3; k++) {
                    jacob_n[3*j + k] = 0.0;
                }
            } else {
                const cmpl dR[2] = {mlr_jacob_n[j], mlr_jacob_n[nb+j]};
                cmpl dsp[3];
                sp_products_der(R, dR, tanlz, dsp);
                for (int k = 0; k < 3; k++) {
                    jacob_n[3*j + k] = dsp[k];
                }
            }
        }
    }

    if(jacob_th) {
        for(int j = 0; j < nblyr; j++) {
            cmpl dsp[3];
            cmpl dR[2] = {mlr_jacob_th[j], mlr_jacob_th[nblyr+j]};
            sp_products_der(R, dR, tanlz, dsp);
            for (int k = 0; k < 3; k++) {
                jacob_th[3*j + k] = creal(dsp[k]);
            }
        }
    }

    if (dsp_daoi) {
        cmpl dsp[3];
        sp_products_der(R, dRdaoi, tanlz, dsp);
        for (int k = 0; k < 3; k++) {
            dsp_daoi[k] = deg_to_radians(creal(dsp[k]));
        }
    }
}

/* Legendre-Gauss quadrature abscissae and coefficients. From:
   http://pomax.github.io/bezierinfo/legendre-gauss.html */
static double gauss_quad_5_x[5] = {0.0,                0.5384693101056831, 0.9061798459386640};
static double gauss_quad_5_w[5] = {0.5688888888888889, 0.4786286704993665, 0.2369268850561891};

void
mult_layer_se_bandwidth_jacob(enum se_type type,
                    size_t _nb, const cmpl ns[], double phi0,
                    const double ds[], double lambda,
                    double anlz, const double bandwidth, ell_ab_t e,
                    gsl_vector *jacob_th, cmpl_vector *jacob_n, double *jacob_acq)
{
    const int nb = _nb, nblyr = nb - 2;
    const double tanlz = tan(anlz);
    const cmpl nsin0 = ns[0] * csin((cmpl) phi0);
    double jacob_th_sum[3 * nblyr];
    cmpl jacob_n_sum[3 * nb];
    double dsp_daoi[3];
    double sp[3] = {0.0, 0.0, 0.0};

    if (jacob_th) {
        for (int j = 0; j < nblyr; j++) {
            jacob_th_sum[j] = 0.0;
        }
    }

    if (jacob_n) {
        for (int j = 0; j < nb; j++) {
            jacob_n_sum[j] = 0.0;
        }
    }

    if (jacob_acq) {
        for (int k = 0; k < 3; k++) {
            dsp_daoi[k] = 0.0;
        }
    }

    const int ORDER = 5;
    for (int i = 0; i < ORDER; i++) {
        const int qc_index = (i >= 2 ? i - 2 : 2 - i);
        double sp_w[3], jacob_th_w[nblyr], dsp_daoi_w[3];
        cmpl jacob_n_w[nb];

        const double lambda_w = lambda + gauss_quad_5_x[qc_index] * bandwidth / 2;
        mult_layer_sp_products_jacob(nb, nsin0, ns, ds, lambda_w, tanlz, sp_w, jacob_th_w, jacob_n_w, dsp_daoi_w);

        const double weight = gauss_quad_5_w[qc_index];
        for (int k = 0; k < 3; k++) {
            sp[k] += weight * sp_w[k];
        }

        if (jacob_th) {
            for (int j = 0; j < 3 * nblyr; j++) {
                jacob_th_sum[j] += weight * jacob_th_w[j];
            }
        }

        if (jacob_n) {
            for (int j = 0; j < 3 * nb; j++) {
                jacob_n_sum[j] += weight * jacob_n_w[j];
            }
        }

        if (jacob_acq) {
            for (int k = 0; k < 3; k++) {
                dsp_daoi[k] += weight * dsp_daoi_w[k];
            }
        }
    }

    if (type == SE_ALPHA_BETA) {
        const double tasq = tanlz * tanlz;
        const cmpl abden = sp[0] + tasq * sp[1];
        const cmpl densq = sqr(abden);

        e->alpha = (sp[0] - tasq * sp[1]) / abden;
        e->beta = (2 * sp[2] * tanlz) / abden;

        if (jacob_n) {
            for(int j = 0; j < nb; j++) {
                const cmpl dRs2 = jacob_n_sum[3*j], dRp2 = jacob_n_sum[3*j+1], dRsRpcj = jacob_n_sum[3*j+2];
                const cmpl d_alpha = 2 * tasq / densq * (sp[1] * dRs2 - sp[0] * dRp2);
                const cmpl d_beta = 2 * tanlz / densq * (- sp[2] * dRs2 - sp[2] * tasq * dRp2 + dRsRpcj);
                cmpl_vector_set(jacob_n, j,      d_alpha);
                cmpl_vector_set(jacob_n, nb + j, d_beta );
            }
        }

        if (jacob_th) {
            for(int j = 0; j < nblyr; j++) {
                const cmpl dRs2 = jacob_th_sum[3*j], dRp2 = jacob_th_sum[3*j+1], dRsRpcj = jacob_th_sum[3*j+2];
                const cmpl d_alpha = 2 * tasq / densq * (sp[1] * dRs2 - sp[0] * dRp2);
                const cmpl d_beta = 2 * tanlz / densq * (- sp[2] * dRs2 - sp[2] * tasq * dRp2 + dRsRpcj);
                gsl_vector_set(jacob_th, j,         creal(d_alpha));
                gsl_vector_set(jacob_th, nblyr + j, creal(d_beta) );
            }
        }

        if (jacob_acq) {
            const cmpl dRs2 = dsp_daoi[0], dRp2 = dsp_daoi[1], dRsRpcj = dsp_daoi[2];
            const cmpl d_alpha = 2 * tasq / densq * (sp[1] * dRs2 - sp[0] * dRp2);
            const cmpl d_beta = 2 * tanlz / densq * (- sp[2] * dRs2 - sp[2] * tasq * dRp2 + dRsRpcj);
            /* Derivatives with AOI. */
            jacob_acq[SE_ACQ_INDEX(SE_ALPHA, SE_AOI)] = creal(d_alpha);
            jacob_acq[SE_ACQ_INDEX(SE_BETA , SE_AOI)] = creal(d_beta);

            /* Derivatives with Analyzer angle (A). */
            const double secsqa = 1 + tasq; /* = 1 / cos^2(A) = sec^2(A) */
            jacob_acq[SE_ACQ_INDEX(SE_ALPHA, SE_ANALYZER)] = - 4 * sp[0] * sp[1] * tasq * secsqa / densq;
            jacob_acq[SE_ACQ_INDEX(SE_BETA , SE_ANALYZER)] = 2 * sp[2] * (sp[0] - tasq * sp[1]) * secsqa / densq;
        }
    } else {
        const double tpsi = sqrt(sp[0] / sp[1]);
        e->alpha = tpsi;
        e->beta = sp[2] / tpsi;

        /* TO BE FINISHED */
    }
}
