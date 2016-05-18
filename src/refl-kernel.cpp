#include "refl-kernel.h"

const double PI = 3.14159265358979323846;
const auto I = std::complex<double>(0.0, 1.0);

typedef std::complex<double> complex;

static complex csqr(const complex x) { return x*x; }

static complex
refl_coeff_ni(const complex nt, const complex nb) {
    return (nb - nt) / (nb + nt);
}

static complex
refl_coeff_ext_ni(complex nt, complex nb, complex *drdnt, complex *drdnb)
{
    complex aux = 1.0 / (nb + nt);
    complex r = (nb - nt) * aux;
    aux *= aux;
    *drdnt = - 2.0 * nb * aux;
    *drdnb =   2.0 * nt * aux;
    return r;
}

static complex
mult_layer_refl_ni_nojacob(int nb, const complex ns[], const double ds[],
                           double lambda)
{
    const double omega = 2 * PI / lambda;

    complex nt = ns[nb-2];
    complex nc = ns[nb-1];

    complex R = refl_coeff_ni(nt, nc);

    for(int j = nb - 3; j >= 0; j--) {
        const double th = ds[j];

        nc = nt;
        nt = ns[j];

        const complex beta = - 2.0 * I * omega * nc;
        const complex rho = std::exp(beta * THICKNESS_TO_NM(th));

        const complex r = refl_coeff_ni(nt, nc);

        const complex den = 1.0 + r * R * rho;
        R = (r + R * rho) / den;
    }

    return R;
}

static complex
mult_layer_refl_ni_jacob_th(int nb, const complex ns[], const double ds[],
                            double lambda, complex *jacth)
{
    const double omega = 2 * PI / lambda;
    const int nblyr = nb - 2;

    complex nt = ns[nb-2];
    complex nc = ns[nb-1];

    complex R = refl_coeff_ni(nt, nc);

    for(int j = nb - 3; j >= 0; j--) {
        const double th = ds[j];

        nc = nt;
        nt = ns[j];

        const complex beta = - 2.0 * I * omega * nc;
        const complex rho = std::exp(beta * THICKNESS_TO_NM(th));
        const complex drhodth = rho * beta * THICKNESS_TO_NM(1.0);

        const complex r = refl_coeff_ni(nt, nc);

        const complex den = 1.0 + r * R * rho;
        const complex isqden = 1.0 / csqr(den);
        const complex dfdR = rho * (1.0 - r*r) * isqden;

        for(int k = nblyr; k > j+1; k--) {
            jacth[k-1] *= dfdR;
        }

        const complex dfdrho = R * (1.0 - r*r) * isqden;

        jacth[j] = dfdrho * drhodth;

        R = (r + R * rho) / den;
    }

    return R;
}

static complex
mult_layer_refl_ni_jacob(int nb, const complex ns[], const double ds[],
                         double lambda, complex *jacth, complex *jacn)
{
    const double omega = 2 * PI / lambda;
    const int nblyr = nb - 2;
    complex drdnt, drdnb;

    complex nt = ns[nb-2];
    complex nc = ns[nb-1];

    complex R = refl_coeff_ext_ni(nt, nc, &drdnt, &drdnb);

    jacn[nb-1] = drdnb;
    jacn[nb-2] = drdnt;

    for(int j = nb - 3; j >= 0; j--) {
        const double th = ds[j];

        nc = nt;
        nt = ns[j];

        const complex beta = - 2.0 * I * omega * nc;
        const complex rho = std::exp(beta * THICKNESS_TO_NM(th));
        const complex drhodth = rho * beta * THICKNESS_TO_NM(1.0);
        const complex drhodn = - 2.0 * I * rho * omega * THICKNESS_TO_NM(th);

        const complex r = refl_coeff_ext_ni(nt, nc, &drdnt, &drdnb);

        const complex den = 1.0 + r * R * rho;
        const complex isqden = 1.0 / csqr(den);
        const complex dfdR = rho * (1.0 - r*r) * isqden;

        for(int k = nb - 1; k > j+1; k--) {
            jacn[k] *= dfdR;
        }

        for(int k = nblyr; k > j+1; k--) {
            jacth[k-1] *= dfdR;
        }

        const complex dfdr = (1.0 - csqr(R*rho)) * isqden;
        const complex dfdrho = R * (1.0 - r*r) * isqden;

        jacn[j+1] = dfdR * jacn[j+1] + dfdr * drdnb + dfdrho * drhodn;
        jacn[j] = dfdr * drdnt;

        jacth[j] = dfdrho * drhodth;

        R = (r + R * rho) / den;
    }

    return R;
}

double
mult_layer_refl_ni(size_t _nb, const complex ns[], const double ds[],
                   double lambda,
                   gsl_vector *r_jacob_th, gsl_vector *r_jacob_n)
{
    complex *jacobian_th, *jacobian_n;
    int nb = _nb;
    complex r;

    jacobian_th = new complex[nb];
    jacobian_n  = new complex[nb];

    if(r_jacob_th && r_jacob_n) {
        r = mult_layer_refl_ni_jacob(nb, ns, ds, lambda, jacobian_th, jacobian_n);
    } else if(r_jacob_th) {
        r = mult_layer_refl_ni_jacob_th(nb, ns, ds, lambda, jacobian_th);
    } else {
        r = mult_layer_refl_ni_nojacob(nb, ns, ds, lambda);
    }

    if(r_jacob_th) {
        for(int k = 0; k < nb-2; k++) {
            complex dr = jacobian_th[k];
            double drsq = 2 * (r.real()*dr.real() + r.imag()*dr.imag());
            gsl_vector_set(r_jacob_th, k, drsq);
        }
    }

    if(r_jacob_n) {
        for(int k = 0; k < nb; k++) {
            complex dr = jacobian_n[k];
            double drsqr = 2 * (r.real()*dr.real() + r.imag()*dr.imag());
            double drsqi = 2 * (r.imag()*dr.real() - r.real()*dr.imag());
            gsl_vector_set(r_jacob_n, k, drsqr);
            gsl_vector_set(r_jacob_n, nb + k, drsqi);
        }
    }

    delete [] jacobian_th;
    delete [] jacobian_n;

    return norm(r);
}
