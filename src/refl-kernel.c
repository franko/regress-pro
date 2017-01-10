#include <assert.h>

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
                   gsl_vector *r_jacob_th, gsl_vector *r_jacob_n)
{
    cmpl mlr_jacob_th[nb], mlr_jacob_n[nb];
    size_t k;
    cmpl r;

    assert(nb >= 2);

    if(r_jacob_th && r_jacob_n) {
        r = mult_layer_refl_ni_jacob(nb, ns, ds, lambda, mlr_jacob_th, mlr_jacob_n);
    } else if(r_jacob_th) {
        r = mult_layer_refl_ni_jacob_th(nb, ns, ds, lambda, mlr_jacob_th);
    } else {
        r = mult_layer_refl_ni_nojacob(nb, ns, ds, lambda);
    }

    if(r_jacob_th)
        for(k = 0; k < (size_t) nb-2; k++) {
            cmpl dr = mlr_jacob_th[k];
            double drsq = 2 * (creal(r)*creal(dr) + cimag(r)*cimag(dr));
            gsl_vector_set(r_jacob_th, k, drsq);
        }

    if(r_jacob_n)
        for(k = 0; k < (size_t) nb; k++) {
            cmpl dr = mlr_jacob_n[k];
            double drsqr = 2 * (creal(r)*creal(dr) + cimag(r)*cimag(dr));
            double drsqi = 2 * (cimag(r)*creal(dr) - creal(r)*cimag(dr));
            gsl_vector_set(r_jacob_n, k, drsqr);
            gsl_vector_set(r_jacob_n, nb + k, drsqi);
        }

    return CSQABS(r);
}

static double
mult_layer_refl_ni_bandwidth(int nb, const cmpl ns[], const double ds[],
                             double lambda, const double bandwidth,
                             gsl_vector *r_jacob_th, gsl_vector *r_jacob_n)
{
    cmpl mlr_jacob_th[nb], mlr_jacob_n[nb];
    double rsq = 0.0;

    if(r_jacob_th) {
        gsl_vector_set_zero(r_jacob_th);
    }

    if(r_jacob_n) {
        gsl_vector_set_zero(r_jacob_n);
    }

    const struct gauss_quad_info *quad_rule = gauss_rule(GAUSS_LEGENDRE_RULE_7);
    for (int i = -quad_rule->n; i <= quad_rule->n; i++) {
        const double rule_abscissa = gauss_rule_abscissa(quad_rule, i);
        const double lambda_w = lambda + rule_abscissa * bandwidth / 2.0;
        cmpl r;

        if(r_jacob_th && r_jacob_n) {
            r = mult_layer_refl_ni_jacob(nb, ns, ds, lambda_w, mlr_jacob_th, mlr_jacob_n);
        } else if(r_jacob_th) {
            r = mult_layer_refl_ni_jacob_th(nb, ns, ds, lambda_w, mlr_jacob_th);
        } else {
            r = mult_layer_refl_ni_nojacob(nb, ns, ds, lambda_w);
        }

        const double weight = 0.5 * gauss_rule_weigth(quad_rule, i);
        rsq += weight * CSQABS(r);

        if(r_jacob_th) {
            for(int k = 0; k < nb-2; k++) {
                const cmpl dr = mlr_jacob_th[k];
                const double drsq = 2 * (creal(r)*creal(dr) + cimag(r)*cimag(dr));
                const double jv = gsl_vector_get(r_jacob_th, k) + weight * drsq;
                gsl_vector_set(r_jacob_th, k, jv);
            }
        }

        if(r_jacob_n) {
            for(int k = 0; k < nb; k++) {
                const cmpl dr = mlr_jacob_n[k];
                const double drsqr = 2 * (creal(r)*creal(dr) + cimag(r)*cimag(dr));
                const double drsqi = 2 * (cimag(r)*creal(dr) - creal(r)*cimag(dr));
                const double jvr = gsl_vector_get(r_jacob_n, k) + weight * drsqr;
                const double jvi = gsl_vector_get(r_jacob_n, nb + k) + weight * drsqi;
                gsl_vector_set(r_jacob_n, k, jvr);
                gsl_vector_set(r_jacob_n, nb + k, jvi);
            }
        }
    }

    return rsq;
}

double
mult_layer_refl_sr(size_t nb, const cmpl ns[], const double ds[],
                   double lambda, const struct acquisition_parameters *acquisition,
                   gsl_vector *jacob_th, gsl_vector *jacob_n)
{
    if (acquisition->bandwidth > 0.0) {
        return mult_layer_refl_ni_bandwidth(nb, ns, ds, lambda, acquisition->bandwidth, jacob_th, jacob_n);
    }
    return mult_layer_refl_ni(nb, ns, ds, lambda, jacob_th, jacob_n);
}
