
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
#include <string.h>

#include "elliss.h"
#include "gauss-legendre-quad.h"

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
    jac[SE_ACQ_INDEX(SE_TANPSI, SE_AOI)] = deg_to_radians(creal(iden * conj(rho) * drho));
    jac[SE_ACQ_INDEX(SE_COSDEL, SE_AOI)] = deg_to_radians(creal(iden * (1 - conj(rho) / rho) * drho / 2.0));
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
    jac[SE_ACQ_INDEX(SE_ALPHA, SE_AOI)] = deg_to_radians(creal(4 * tasq * z1 * isqden));
    jac[SE_ACQ_INDEX(SE_BETA , SE_AOI)] = deg_to_radians(creal(2 * tanlz * z2 * isqden));

    /* Derivatives with Analyzer angle (A). */
    const double secsqa = 1 + tasq; /* = 1 / cos^2(A) = sec^2(A) */
    jac[SE_ACQ_INDEX(SE_ALPHA, SE_ANALYZER)] = deg_to_radians(- 4 * tanlz * sqtpsi * isqden * secsqa);
    jac[SE_ACQ_INDEX(SE_BETA , SE_ANALYZER)] = deg_to_radians(2 * creal(rho) * (sqtpsi - tasq) * isqden * secsqa);
}

static void
set_se_measuring_jacobian_real(enum se_type se_type, const int n, cmpl R[2], const double tanlz, const cmpl mlr_jacob[], double jacob[])
{
    for(int j = 0; j < n; j++) {
        cmpl d_alpha, d_beta;
        cmpl dR[2] = {mlr_jacob[j], mlr_jacob[n + j]};
        if(se_type == SE_ALPHA_BETA) {
            se_ab_der(R, dR, tanlz, &d_alpha, &d_beta);
        } else {
            se_psidel_der(R, dR, &d_alpha, &d_beta);
        }
        jacob[j    ] = creal(d_alpha);
        jacob[n + j] = creal(d_beta);
    }
}

static void
set_se_measuring_jacobian_complex(enum se_type se_type, const int n, cmpl R[2], const double tanlz, const cmpl mlr_jacob[], cmpl jacob[])
{
    for(int j = 0; j < n; j++) {
        cmpl d_alpha, d_beta;
        cmpl dR[2] = {mlr_jacob[j], mlr_jacob[n + j]};
        if(se_type == SE_ALPHA_BETA) {
            se_ab_der(R, dR, tanlz, &d_alpha, &d_beta);
        } else {
            se_psidel_der(R, dR, &d_alpha, &d_beta);
        }
        jacob[j    ] = d_alpha;
        jacob[n + j] = d_beta;
    }

}

static void
mult_layer_se_jacob(enum se_type type,
                    int nb, const cmpl ns[], double phi0,
                    const double ds[], double lambda,
                    double anlz, ell_ab_t e,
                    double *jacob_th, cmpl *jacob_n, double *jacob_acquisition)
{
    const int nblyr = nb - 2;
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
    }
}

static void
sp_products(const cmpl R[2], double tanlz, double sp[3])
{
    sp[0] = CSQABS(R[0]); /* |Rs|^2 */
    sp[1] = CSQABS(R[1]); /* |Rp|^2 */
    sp[2] = creal(R[1] * conj(R[0])); /* Re(Rp Rs*) */
}

static void
sp_products_diff(const cmpl R[2], const cmpl dR[2], double tanlz, cmpl dsp[3])
{
    dsp[0] = 2 * R[0] * conj(dR[0]); /* derivative of |Rs|^2 */
    dsp[1] = 2 * R[1] * conj(dR[1]); /* derivative of |Rp|^2 */
    dsp[2] = R[0] * conj(dR[1]) + R[1] * conj(dR[0]); /* derivative of Re(Rp Rs*) */
}

static void
sp_products_diff_real(const cmpl R[2], const cmpl dR[2], double tanlz, double dsp[3])
{
    cmpl dsp_complex[3];
    sp_products_diff(R, dR, tanlz, dsp_complex);
    for (int k = 0; k < 3; k++) {
        dsp[k] = creal(dsp_complex[k]);
    }
}

static void
se_ab_diff_from_sp(const double sp[3], const cmpl dsp[3], const double tanlz, cmpl dab[2]) {
    const double tasq = tanlz * tanlz;
    const double abden = sp[1] + tasq * sp[0];
    const cmpl drs2 = dsp[0], drp2 = dsp[1], drprs = dsp[2];
    dab[0] = 2 * tasq / sqr(abden) * (sp[0] * drp2 - sp[1] * drs2);
    dab[1] = 2 * tanlz / abden * (drprs - sp[2] * (drp2 + tasq * drs2) / abden);
}

static void
se_ab_diff_from_sp_real(const double sp[3], const double dsp[3], const double tanlz, double dab[2]) {
    const double tasq = tanlz * tanlz;
    const double abden = sp[1] + tasq * sp[0];
    const double drs2 = dsp[0], drp2 = dsp[1], drprs = dsp[2];
    dab[0] = 2 * tasq / sqr(abden) * (sp[0] * drp2 - sp[1] * drs2);
    dab[1] = 2 * tanlz / abden * (drprs - sp[2] * (drp2 + tasq * drs2) / abden);
}

static void
se_psidel_diff_from_sp(const double sp[3], const cmpl dsp[3], cmpl dpsidel[2]) {
    const double tan_psi = sqrt(sp[1] / sp[0]); /* tan(psi) = SQRT(|Rp|^2/|Rs|^2) */
    const double cos_delta = sp[2] / sqrt(sp[0] * sp[1]); /* cos(delta) = Re(Rp Rs*) / (|Rs| |Rp|) */
    const cmpl drs2 = dsp[0], drp2 = dsp[1], drprs = dsp[2];
    dpsidel[0] = tan_psi / 2.0 * (drp2 / sp[1] - drs2 / sp[0]) ;
    dpsidel[1] = cos_delta / 2.0 * (- drp2 / sp[1] - drs2 / sp[0] + 2.0 / sp[2] * drprs);
}

static void
se_psidel_diff_from_sp_real(const double sp[3], const double dsp[3], double dpsidel[2]) {
    const double tan_psi = sqrt(sp[1] / sp[0]); /* tan(psi) = SQRT(|Rp|^2/|Rs|^2) */
    const double cos_delta = sp[2] / sqrt(sp[0] * sp[1]); /* cos(delta) = Re(Rp Rs*) / (|Rs| |Rp|) */
    const double drs2 = dsp[0], drp2 = dsp[1], drprs = dsp[2];
    dpsidel[0] = tan_psi / 2.0 * (drp2 / sp[1] - drs2 / sp[0]) ;
    dpsidel[1] = cos_delta / 2.0 * (- drp2 / sp[1] - drs2 / sp[0] + 2.0 / sp[2] * drprs);
}

static void
se_ab_set_jacob_acquisition_from_sp(const double sp[3], const double sp_diff[3], double *jacob_acq, int se_parameter, const double tanlz) {
    double dab[2];
    se_ab_diff_from_sp_real(sp, sp_diff, tanlz, dab);
    jacob_acq[SE_ACQ_INDEX(SE_ALPHA, se_parameter)] = dab[0];
    jacob_acq[SE_ACQ_INDEX(SE_BETA , se_parameter)] = dab[1];
}

static void
se_psidel_set_jacob_acquisition_from_sp(const double sp[3], const double sp_diff[3], double *jacob_acq, int se_parameter) {
    double dpsidel[2];
    se_psidel_diff_from_sp_real(sp, sp_diff, dpsidel);
    jacob_acq[SE_ACQ_INDEX(SE_ALPHA, se_parameter)] = dpsidel[0];
    jacob_acq[SE_ACQ_INDEX(SE_BETA , se_parameter)] = dpsidel[1];
}

static void
mult_layer_se_integ_jacob(int nb, const cmpl ns[], const double ds[], double lambda,
                    const struct acquisition_parameters *acquisition, ell_ab_t e,
                    double *jacob_th, cmpl *jacob_n, double *jacob_acq)
{
    const double phi0 = deg_to_radians(acquisition_get_se_aoi(acquisition));
    const double anlz = deg_to_radians(acquisition_get_se_analyzer(acquisition));
    const int se_type = SE_TYPE(acquisition->type);
    const int nblyr = nb - 2;
    const double tanlz = tan(anlz);
    double sp_jacob_th[3 * nblyr];
    cmpl sp_jacob_n[3 * nb];
    double sp_diff_aoi[3] = {0.0};
    double sp_diff_numap[3] = {0.0};
    double sp_diff_bandwidth[3] = {0.0};
    double sp[3] = {0.0};

    memset(sp_jacob_th, 0, 3 * nblyr * sizeof(double));
    memset(sp_jacob_n, 0, 3 * nb * sizeof(cmpl));

    const struct gauss_quad_info *quad_rule_bw = (acquisition->bandwidth > 0.0 ? gauss_rule(GAUSS_LEGENDRE_RULE_7) : gauss_rule(UNIT_RULE));
    const struct gauss_quad_info *quad_rule_na = (acquisition->numap > 0.0 ? gauss_rule(GAUSS_LEGENDRE_RULE_3) : gauss_rule(UNIT_RULE));
    const double delta_phi0 = asin(acquisition->numap);
    const double icosdphi = 1.0 / cos(delta_phi0);
    for (int ibw = -quad_rule_bw->n; ibw <= quad_rule_bw->n; ibw++) {
        for (int ina = -quad_rule_na->n; ina <= quad_rule_na->n; ina++) {
            const double rule_abscissa_bw = gauss_rule_abscissa(quad_rule_bw, ibw);
            const double rule_abscissa_na = gauss_rule_abscissa(quad_rule_na, ina);
            const double lambda_bw = lambda + rule_abscissa_bw * acquisition->bandwidth / 2.0;
            const cmpl nsin0_na = ns[0] * csin((cmpl) (phi0 + rule_abscissa_na * delta_phi0));
            cmpl mlr_jacob_th[2 * nblyr], mlr_jacob_n[2 * nb];
            cmpl R[2], dRdaoi[2];

            if(jacob_th && jacob_n) {
                mult_layer_refl_jacob(nb, ns, nsin0_na, ds, lambda_bw, R, dRdaoi, mlr_jacob_th, mlr_jacob_n);
            } else if (jacob_th || jacob_acq) {
                mult_layer_refl_jacob_th_aoi(nb, ns, nsin0_na, ds, lambda_bw, R, dRdaoi, mlr_jacob_th);
            } else if (jacob_th) {
                mult_layer_refl_jacob_th(nb, ns, nsin0_na, ds, lambda_bw, R, mlr_jacob_th);
            } else {
                mult_layer_refl(nb, ns, nsin0_na, ds, lambda_bw, R);
            }

            /* Compute into sp_w the Rp Rs products for the adjusted wavelength. */
            double sp_w[3];
            sp_products(R, tanlz, sp_w);

            const double weight = (0.5 * gauss_rule_weigth(quad_rule_bw, ibw)) * (0.5 * gauss_rule_weigth(quad_rule_na, ina));
            for (int k = 0; k < 3; k++) {
                sp[k] += weight * sp_w[k];
            }

            if (jacob_n) {
                for (int j = 0; j < nb; j++) {
                    const cmpl dR[2] = {mlr_jacob_n[j], mlr_jacob_n[nb+j]};
                    cmpl dsp[3];
                    sp_products_diff(R, dR, tanlz, dsp);
                    /* There is a quirk here. We take the conjugate because
                       in the old code that differentiate based on the formula for
                       rho the derivatives are the conjugate of what is taken from
                       the Rp Rs expression we use here.
                       So we take the conjugate to align with the old code. */
                    for (int k = 0; k < 3; k++) {
                        sp_jacob_n[3*j + k] += weight * conj(dsp[k]);
                    }
                }
            }

            /* We store the derivatives with thickness for each layer. These are
               required to compute the derivatives with lambda to calculate, in turn,
               the derivative with the bandwidth. */
            double dspdt[nblyr * 3];
            if (jacob_th || jacob_acq) {
                for (int j = 0; j < nblyr; j++) {
                    const cmpl dR[2] = {mlr_jacob_th[j], mlr_jacob_th[nblyr+j]};
                    sp_products_diff_real(R, dR, tanlz, dspdt + 3*j);
                    for (int k = 0; k < 3; k++) {
                        sp_jacob_th[3*j + k] += weight * dspdt[3*j + k];
                    }
                }
            }

            if (jacob_acq) {
                double dsp[3];
                sp_products_diff_real(R, dRdaoi, tanlz, dsp);

                for (int k = 0; k < 3; k++) {
                    sp_diff_aoi[k] += weight * deg_to_radians(dsp[k]);
                    /* Since NA = sin(delta_phi0) to obtain the derivative with NA we need to multiply the derivative with delta_phi0 by 1 / cos(delta_phi0). */
                    sp_diff_numap[k] += icosdphi * weight * rule_abscissa_na * dsp[k];

                    /* Compute first the derivative of each Rp Rs product wrt to
                       lambda (the wavelength). Done using the derivatives with
                       the films' thicknesses. */
                    double dpdlambda = 0.0;
                    for (int j = 0; j < nblyr; j++) {
                        dpdlambda += - dspdt[3*j + k] * ds[j] / lambda_bw;
                    }
                    sp_diff_bandwidth[k] += weight * rule_abscissa_bw * dpdlambda / 2.0;
                }
            }
        }
    }

    if (se_type == SE_ALPHA_BETA) {
        const double tasq = tanlz * tanlz;
        const double abden = sp[1] + tasq * sp[0];

        e->alpha = (sp[1] - tasq * sp[0]) / abden;
        e->beta = (2 * sp[2] * tanlz) / abden;

        if (jacob_n) {
            for(int j = 0; j < nb; j++) {
                cmpl dab[2];
                se_ab_diff_from_sp(sp, sp_jacob_n + 3 * j, tanlz, dab);
                jacob_n[     j] = dab[0];
                jacob_n[nb + j] = dab[1];
            }
        }

        if (jacob_th) {
            for(int j = 0; j < nblyr; j++) {
                double dab[2];
                se_ab_diff_from_sp_real(sp, sp_jacob_th + 3 * j, tanlz, dab);
                jacob_th[        j] = dab[0];
                jacob_th[nblyr + j] = dab[1];
            }
        }

        if (jacob_acq) {
            se_ab_set_jacob_acquisition_from_sp(sp, sp_diff_aoi,       jacob_acq, SE_AOI,       tanlz);
            se_ab_set_jacob_acquisition_from_sp(sp, sp_diff_numap,     jacob_acq, SE_NUMAP,     tanlz);
            se_ab_set_jacob_acquisition_from_sp(sp, sp_diff_bandwidth, jacob_acq, SE_BANDWIDTH, tanlz);

            /* Derivatives with Analyzer angle (A). */
            const double secsqa = 1 + tasq; /* = 1 / cos^2(A) = sec^2(A) */
            const double iden = 1 / sqr(abden);
            jacob_acq[SE_ACQ_INDEX(SE_ALPHA, SE_ANALYZER)] = deg_to_radians(- 4 * sp[0] * sp[1] * tanlz * secsqa * iden);
            jacob_acq[SE_ACQ_INDEX(SE_BETA , SE_ANALYZER)] = deg_to_radians(2 * sp[2] * (sp[1] - tasq * sp[0]) * secsqa * iden);
        }
    } else {
        e->alpha = sqrt(sp[1] / sp[0]); /* tan(psi) = SQRT(|Rp|^2/|Rs|^2) */
        e->beta = sp[2] / sqrt(sp[0] * sp[1]); /* cos(delta) = Re(Rp Rs*) / (|Rs| |Rp|) */

        if (jacob_n) {
            for(int j = 0; j < nb; j++) {
                cmpl dpsidel[2];
                se_psidel_diff_from_sp(sp, sp_jacob_n + 3 * j, dpsidel);
                jacob_n[     j] = dpsidel[0];
                jacob_n[nb + j] = dpsidel[1];
            }
        }

        if (jacob_th) {
            for(int j = 0; j < nblyr; j++) {
                double dpsidel[2];
                se_psidel_diff_from_sp_real(sp, sp_jacob_th + 3 * j, dpsidel);
                jacob_th[        j] = dpsidel[0];
                jacob_th[nblyr + j] = dpsidel[1];
            }
        }

        if (jacob_acq) {
            se_psidel_set_jacob_acquisition_from_sp(sp, sp_diff_aoi,       jacob_acq, SE_AOI);
            se_psidel_set_jacob_acquisition_from_sp(sp, sp_diff_numap,     jacob_acq, SE_NUMAP);
            se_psidel_set_jacob_acquisition_from_sp(sp, sp_diff_bandwidth, jacob_acq, SE_BANDWIDTH);
        }
    }
}

void
mult_layer_refl_se(enum se_type se_type,
                   size_t nb, const cmpl ns[],
                   const double ds[], double lambda,
                   const struct acquisition_parameters *acquisition, ell_ab_t e,
                   double *jacob_th, cmpl *jacob_n, double *jacob_acq)
{
    if (acquisition->bandwidth > 0.0 || acquisition->numap > 0.0 || jacob_acq) {
        mult_layer_se_integ_jacob(nb, ns, ds, lambda, acquisition, e, jacob_th, jacob_n, jacob_acq);
    } else {
        const double phi0 = deg_to_radians(acquisition_get_se_aoi(acquisition));
        const double anlz = deg_to_radians(acquisition_get_se_analyzer(acquisition));
        mult_layer_se_jacob(se_type, nb, ns, phi0, ds, lambda, anlz, e, jacob_th, jacob_n, jacob_acq);
    }
}
