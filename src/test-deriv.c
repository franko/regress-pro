#ifdef DEBUG_REGRESS

#include <stdio.h>
#include <string.h>
#include <gsl/gsl_deriv.h>

#include "test-deriv.h"

struct aux_param {
    int layer;
    int thickness;
    int real_part;
    int channel;

    double *ds;
    cmpl *ns;

    enum se_type spkind;
    size_t nb;
    double phi0;
    double lambda;
    double anlz;
};

static double der_aux_f(double x, void *_p);

double
der_aux_f(double x, void *_p)
{
    struct aux_param *p = (struct aux_param *) _p;
    ell_ab_t e;

    if(p->thickness) {
        p->ds[p->layer-1] = x;
    } else {
        if(p->real_part) {
            cmpl z = p->ns[p->layer];
            p->ns[p->layer] = x + I * cimag(z);
        } else {
            cmpl z = p->ns[p->layer];
            p->ns[p->layer] = creal(z) + I * x;
        }
    }

    mult_layer_se_jacob(p->spkind,
                        p->nb, p->ns, p->phi0, p->ds, p->lambda, p->anlz,
                        e, NULL, NULL);

    return (p->channel == 0 ? e->alpha : e->beta);
}

void
test_elliss_deriv(enum se_type spkind,
                  size_t _nb, const cmpl ns[], double phi0,
                  const double ds[], double lambda,
                  double anlz)
{
    gsl_vector *jacob_th;
    cmpl_vector *jacob_n;
    struct aux_param p[1];
    size_t j, nb = _nb;
    size_t nblyr = nb - 2;
    double *myds;
    cmpl *myns;
    ell_ab_t e;
    size_t noff;

#define TEST_CHANNEL 1

    printf("LAMBDA: %f\n", lambda);

    jacob_th = gsl_vector_alloc(2 * nblyr);
    jacob_n  = cmpl_vector_alloc(2 * nb);

    myds = emalloc(nblyr * sizeof(double));
    myns = emalloc(nb * sizeof(cmpl));

    mult_layer_se_jacob(spkind, nb, ns, phi0, ds, lambda, anlz,
                        e, jacob_th, jacob_n);

    p->nb = nb;
    p->spkind = spkind;
    p->lambda = lambda;
    p->phi0 = phi0;
    p->anlz = anlz;

    p->ds = myds;
    p->ns = myns;

    p->thickness = 1;
    p->channel = TEST_CHANNEL;
    noff = (p->channel == 0 ? 0 : nblyr);
    for(j = 1; j < nb - 1; j++) {
        gsl_function F;
        double result, abserr;

        p->layer = j;

        F.function = & der_aux_f;
        F.params = p;

        memcpy(myds, ds, nblyr * sizeof(double));
        memcpy(myns, ns, nb * sizeof(cmpl));

        gsl_deriv_central(&F, ds[j-1], 1e-8, &result, &abserr);

        printf("TH layer: %2i, numeric: %.6f, calcul.: %.6f, err: %f\n", j,
               result, gsl_vector_get(jacob_th, noff+j-1), abserr);
    }

    p->thickness = 0;
    p->channel = TEST_CHANNEL;
    noff = (p->channel == 0 ? 0 : nb);
    for(j = 0; j < nb; j++) {
        gsl_function F;
        double result, abserr;

        p->layer = j;

        F.function = & der_aux_f;
        F.params = p;

        memcpy(myds, ds, nblyr * sizeof(double));
        memcpy(myns, ns, nb * sizeof(cmpl));

        p->real_part = 1;
        gsl_deriv_central(&F, creal(ns[j]), 1e-8, &result, &abserr);
        printf("Re{n} layer: %2i, numeric: %.6f, calcul.: %.6f, err: %f\n", j,
               result, creal(cmpl_vector_get(jacob_n, noff+j)), abserr);

        p->real_part = 0;
        gsl_deriv_central(&F, cimag(ns[j]), 1e-8, &result, &abserr);
        printf("Im{n} layer: %2i, numeric: %.6f, calcul.: %.6f, err: %f\n", j,
               result, -cimag(cmpl_vector_get(jacob_n, noff+j)), abserr);
    }

    free(myds), free(myns);
    gsl_vector_free(jacob_th);
    cmpl_vector_free(jacob_n);
}

#endif
