/*
  $Id$
 */

#include <assert.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_blas.h>

#include "lmfit.h"
#include "grid-search.h"
#include "stack.h"
#include "str.h"

int
lmfit_grid(struct fit_engine *fit, struct seeds *seeds,
           double * chisq, str_ptr analysis,
           str_ptr error_msg, int preserve_init_stack,
           gui_hook_func_t hfun, void *hdata)
{
    const gsl_multifit_fdfsolver_type *T;
    gsl_multifit_fdfsolver *s;
    gsl_multifit_function_fdf *f;
    struct fit_config *cfg = fit->config;
    int nb, j, iter, nb_grid_pts, j_grid_pts;
    gsl_vector *x, *xbest;
    double *xarr;
    double chi, chisq_best = 0.0;
    int have_best = 0, have_good = 0;
    int status, stop_request = 0;
    stack_t *initial_stack;
    seed_t *vseed;

    assert(fit->run);

    f = &fit->run->mffun;

    vseed = seeds->values;
    nb    = fit->parameters->number;

    assert(fit->parameters->number == seeds->number);

    x     = gsl_vector_alloc(nb);
    xbest = gsl_vector_alloc(nb);

    if(preserve_init_stack) {
        initial_stack = stack_copy(fit->stack);
    }

    xarr = x->data;

    for(j = 0; j < nb; j++) {
        xarr[j] = (vseed[j].type == SEED_RANGE ? vseed[j].min : vseed[j].seed);
    }

    nb_grid_pts = 1;
    for(j = nb-1; j >= 0; j--) {
        if(vseed[j].type == SEED_RANGE) {
            seed_t *cs = &vseed[j];
            nb_grid_pts *= (cs->max - cs->min) / cs->step + 1;
        }
    }

    if(hfun) {
        (*hfun)(hdata, 0.0, "Running grid search...");
    }

    /* We choose Levenberg-Marquardt algorithm, scaled version*/
    T = gsl_multifit_fdfsolver_lmsder;
    s = gsl_multifit_fdfsolver_alloc(T, f->n, f->p);

    for(j_grid_pts = 0; ; j_grid_pts++) {
        const int search_max_iters = 3;

        gsl_multifit_fdfsolver_set(s, f, x);

        for(j = 0; j < search_max_iters; j++) {
            status = gsl_multifit_fdfsolver_iterate(s);
            if(status != 0) {
                break;
            }
        }

        chi = gsl_blas_dnrm2(s->f);
        *chisq = 1.0E6 * pow(chi, 2.0) / f->n;

        if(!have_best || *chisq < chisq_best) {
            have_best = 1;
            chisq_best = *chisq;
            gsl_vector_memcpy(xbest, x);
        }

        if(*chisq < cfg->chisq_thresold) {
            have_good = 1;
            break;
        }

        if(hfun) {
            float xf = j_grid_pts / (float)nb_grid_pts;
            stop_request = (*hfun)(hdata, xf, NULL);
            if(stop_request) {
                break;
            }
        }

        for(j = nb-1; j >= 0; j--) {
            if(vseed[j].type == SEED_RANGE) {
                xarr[j] += vseed[j].step;
                if(xarr[j] > vseed[j].max) {
                    xarr[j] = vseed[j].min;
                    continue;
                }
                break;
            }
        }

        if(j < 0) {
            break;
        }
    }

    if(j < 0) {
        gsl_vector_memcpy(x, xbest);
    }

    if(stop_request == 0) {
        if(analysis) {
            str_copy_c(analysis, "Seed used: ");
            print_vector(analysis, "%.5g", x);
            str_printf_add(analysis,
                           "With ChiSq: %g. Required thresold was: %g.\n\n",
                           have_good ? *chisq : chisq_best,
                           cfg->chisq_thresold);
        }

        status = lmfit_iter(x, f, s, cfg->nb_max_iters,
                            cfg->epsabs, cfg->epsrel,
                            & iter, hfun, hdata, & stop_request);

        chi = gsl_blas_dnrm2(s->f);
        *chisq = 1.0E6 * pow(chi, 2.0) / f->n;

        if(error_msg) {
            switch(status) {
            case GSL_SUCCESS:
                str_trunc(error_msg, 0);
                break;
            case GSL_CONTINUE:
                str_copy_c(error_msg, "Error: more iterations needed.");
                break;
            default:
                str_printf(error_msg, "Error: %s", gsl_strerror(status));
            }
        }

        if(analysis && !stop_request) {
            str_printf_add(analysis, "Nb of iterations to converge: %i\n",
                           iter);
            print_analysis(analysis, f, s);
        }
    }

    if(stop_request) {
        status = 1;
        if(error_msg) {
            str_copy_c(error_msg, "Fit interrupted by user request.");
        }
        if(analysis) {
            str_copy_c(analysis, "** Fit interrupted by the user.");
        }
    }

    if(preserve_init_stack) {
        /* we restore the initial stack */
        stack_t *tmp_stack = fit->stack;
        fit->stack = initial_stack;
        stack_free(tmp_stack);
    } else {
        /* we take care to commit the results obtained from the fit */
        fit_engine_commit_parameters(fit, x);
    }

    gsl_vector_memcpy(fit->run->results, x);

    gsl_vector_free(x);
    gsl_vector_free(xbest);

    gsl_multifit_fdfsolver_free(s);

    return status;
}
