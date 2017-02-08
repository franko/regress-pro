#include <string.h>

#include "refl-kernel.h"
#include "gauss-legendre-quad.h"

static inline cmpl
csqr(cmpl x)
{
    return x*x;
}

static inline cmpl
refl_coeff_ni(cmpl nt, cmpl nb)
{
    return (nb - nt) / (nb + nt);
}

static cmpl
refl_coeff_ext_ni(cmpl nt, cmpl nb, cmpl * drdnt, cmpl * drdnb)
{
    cmpl aux = 1 / (nb + nt);
    cmpl r = (nb - nt) * aux;
    aux *= aux;
    *drdnt = - 2.0 * nb * aux;
    *drdnb =   2.0 * nt * aux;
    return r;
}

static cmpl
mult_layer_refl_ni_nojacob(int nb, const cmpl ns[], const double ds[],
                           double lambda)
{
    const double omega = 2 * M_PI / lambda;
    cmpl nt, nc, R;
    int j;

    nt = ns[nb-2];
    nc = ns[nb-1];

    R = refl_coeff_ni(nt, nc);

    for(j = nb - 3; j >= 0; j--) {
        cmpl r, rho, beta;
        cmpl den;
        double th = ds[j];

        nc = nt;
        nt = ns[j];

        beta = - 2.0 * I * omega * nc;
        rho = cexp(beta * THICKNESS_TO_NM(th));

        r = refl_coeff_ni(nt, nc);

        den = 1 + r * R * rho;
        R = (r + R * rho) / den;
    }

    return R;
}

static cmpl
mult_layer_refl_ni_jacob_th(int nb, const cmpl ns[], const double ds[],
                            double lambda, cmpl *jacth)
{
    const double omega = 2 * M_PI / lambda;
    const int nblyr = nb - 2;
    cmpl R;
    cmpl nt, nc;
    int j;

    nt = ns[nb-2];
    nc = ns[nb-1];

    R = refl_coeff_ni(nt, nc);

    for(j = nb - 3; j >= 0; j--) {
        cmpl r, rho, beta, drhodth;
        cmpl dfdR, dfdrho;
        cmpl den, isqden;
        double th = ds[j];
        int k;

        nc = nt;
        nt = ns[j];

        beta = - 2.0 * I * omega * nc;
        rho = cexp(beta * THICKNESS_TO_NM(th));
        drhodth = rho * beta * THICKNESS_TO_NM(1.0);

        r = refl_coeff_ni(nt, nc);

        den = 1 + r * R * rho;
        isqden = 1 / csqr(den);
        dfdR = rho * (1 - r*r) * isqden;

        for(k = nblyr; k > j+1; k--) {
            jacth[k-1] *= dfdR;
        }

        dfdrho = R * (1 - r*r) * isqden;

        jacth[j] = dfdrho * drhodth;

        R = (r + R * rho) / den;
    }

    return R;
}

static cmpl
mult_layer_refl_ni_jacob(int nb, const cmpl ns[], const double ds[],
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

    R = refl_coeff_ext_ni(nt, nc, &drdnt, &drdnb);

    jacn[nb-1] = drdnb;
    jacn[nb-2] = drdnt;

    for(j = nb - 3; j >= 0; j--) {
        cmpl r, rho, beta, drhodn, drhodth;
        cmpl dfdR, dfdr, dfdrho;
        cmpl den, isqden;
        double th = ds[j];
        int k;

        nc = nt;
        nt = ns[j];

        beta = - 2.0 * I * omega * nc;
        rho = cexp(beta * THICKNESS_TO_NM(th));
        drhodth = rho * beta * THICKNESS_TO_NM(1.0);
        drhodn = - 2.0 * I * rho * omega * THICKNESS_TO_NM(th);

        r = refl_coeff_ext_ni(nt, nc, &drdnt, &drdnb);

        den = 1 + r * R * rho;
        isqden = 1 / csqr(den);
        dfdR = rho * (1 - r*r) * isqden;

        for(k = nb - 1; k > j+1; k--) {
            jacn[k] *= dfdR;
        }

        for(k = nblyr; k > j+1; k--) {
            jacth[k-1] *= dfdR;
        }

        dfdr = (1 - csqr(R*rho)) * isqden;
        dfdrho = R * (1 - r*r) * isqden;

        jacn[j+1] = dfdR * jacn[j+1] + dfdr * drdnb + dfdrho * drhodn;
        jacn[j] = dfdr * drdnt;

        jacth[j] = dfdrho * drhodth;

        R = (r + R * rho) / den;
    }

    return R;
}

static double
mult_layer_refl_ni(int nb, const cmpl ns[], const double ds[],
                   double lambda,
                   double *jacob_th, cmpl *jacob_n)
{
    cmpl mlr_jacob_th[nb], mlr_jacob_n[nb];
    cmpl r;

    if(jacob_th && jacob_n) {
        r = mult_layer_refl_ni_jacob(nb, ns, ds, lambda, mlr_jacob_th, mlr_jacob_n);
    } else if(jacob_th) {
        r = mult_layer_refl_ni_jacob_th(nb, ns, ds, lambda, mlr_jacob_th);
    } else {
        r = mult_layer_refl_ni_nojacob(nb, ns, ds, lambda);
    }

    if(jacob_th)
        for(int k = 0; k < nb-2; k++) {
            cmpl dr = mlr_jacob_th[k];
            jacob_th[k] = creal(2 * r * conj(dr));
        }

    if(jacob_n)
        for(int k = 0; k < nb; k++) {
            cmpl dr = mlr_jacob_n[k];
            jacob_n[k] = conj(2 * r * conj(dr));
        }

    return CSQABS(r);
}

static double
mult_layer_refl_ni_bandwidth(int nb, const cmpl ns[], const double ds[],
                             double lambda, const double bandwidth,
                             double *jacob_th, cmpl *jacob_n, double *jacob_acq)
{
    cmpl mlr_jacob_th[nb], mlr_jacob_n[nb];
    double rsq_jacob_th[nb - 2];
    double rsq = 0.0;

    memset(rsq_jacob_th, 0, (nb - 2) * sizeof(double));

    if(jacob_n) {
        memset(jacob_n, 0, nb * sizeof(cmpl));
    }

    if (jacob_acq) {
        jacob_acq[SR_BANDWIDTH] = 0.0;
    }

    const struct gauss_quad_info *quad_rule = gauss_rule(GAUSS_LEGENDRE_RULE_7);
    for (int i = -quad_rule->n; i <= quad_rule->n; i++) {
        const double rule_abscissa = gauss_rule_abscissa(quad_rule, i);
        const double lambda_w = lambda + rule_abscissa * bandwidth / 2.0;
        cmpl r;

        if(jacob_th && jacob_n) {
            r = mult_layer_refl_ni_jacob(nb, ns, ds, lambda_w, mlr_jacob_th, mlr_jacob_n);
        } else if(jacob_th) {
            r = mult_layer_refl_ni_jacob_th(nb, ns, ds, lambda_w, mlr_jacob_th);
        } else {
            r = mult_layer_refl_ni_nojacob(nb, ns, ds, lambda_w);
        }

        const double weight = 0.5 * gauss_rule_weigth(quad_rule, i);
        rsq += weight * CSQABS(r);

        if(jacob_n) {
            for(int k = 0; k < nb; k++) {
                const cmpl dr = mlr_jacob_n[k];
                jacob_n[k     ] += weight * conj(2 * r * conj(dr));
            }
        }

        double drsqdt[nb - 2];
        if(jacob_th || jacob_acq) {
            for(int k = 0; k < nb - 2; k++) {
                const cmpl dr = mlr_jacob_th[k];
                rsq_jacob_th[k] += weight * creal(2 * r * conj(dr));
            }
        }

        if (jacob_acq) {
            /* Compute first the derivative of each Rp Rs product wrt to
               lambda (the wavelength). Done using the derivatives with
               the films' thicknesses. */
            double dpdlambda = 0.0;
            for (int k = 0; k < nb - 2; k++) {
                dpdlambda += - drsqdt[k] * ds[k] / lambda_w;
            }
            jacob_acq[SR_BANDWIDTH] += weight * rule_abscissa * dpdlambda / 2.0;
        }
    }

    if (jacob_th) {
        for (int k = 0; k < nb - 2; k++) {
            jacob_th[k] = rsq_jacob_th[k];
        }
    }

    return rsq;
}

int
mult_layer_refl_sr(int nb, const cmpl ns[], const double ds[],
                   double lambda, const struct acquisition_parameters *acquisition,
                   double result[1], double *jacob_th, cmpl *jacob_n, double *jacob_acq)
{
    double r_raw;
    if (acquisition->bandwidth > 0.0 || jacob_acq) {
        r_raw = mult_layer_refl_ni_bandwidth(nb, ns, ds, lambda, acquisition->bandwidth, jacob_th, jacob_n, jacob_acq);
    } else {
        r_raw = mult_layer_refl_ni(nb, ns, ds, lambda, jacob_th, jacob_n);
    }
    result[0] = r_raw * acquisition->parameters.sr.rmult;

    if (jacob_acq) {
        jacob_acq[SR_RMULT] = r_raw;
    }
    return 0;
}
