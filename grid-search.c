#include <assert.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_blas.h>

#include "lmfit.h"
#include "grid-search.h"
#include "stack.h"
#include "fit_result.h"

int
lmfit_grid_run(struct fit_engine *fit, struct seeds *seeds,
    int preserve_init_stack, struct fit_result *result,
    gui_hook_func_t hfun, void *hdata)
{
    const gsl_multifit_fdfsolver_type *T;
    gsl_multifit_fdfsolver *s;
    gsl_multifit_function_fdf *f;
    struct fit_config *cfg = fit->config;
    int nb, j, iter, nb_grid_pts, j_grid_pts;
    gsl_vector *x, *xbest;
    double *xarr;
    double chisq, chi, chisq_best = -1.0;
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

    result->interrupted = 0;
    result->chisq_threshold = cfg->chisq_threshold;
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
        chisq = 1.0E6 * pow(chi, 2.0) / f->n;

        if(chisq_best < 0 || chisq < chisq_best) {
            chisq_best = chisq;
            gsl_vector_memcpy(xbest, x);
        }

        if(chisq < cfg->chisq_threshold) {
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

    /* Case of grid search exhausted or stop request. */
    if(j < 0 || stop_request) {
        gsl_vector_memcpy(x, xbest);
        chisq = chisq_best;
    }

    result->gsearch_chisq = chisq;
    result->chisq = chisq;
    gsl_vector_memcpy(result->gsearch_x, x);
    result->interrupted = stop_request;

    if(stop_request == 0) {
        status = lmfit_iter(x, f, s, cfg->nb_max_iters,
                            cfg->epsabs, cfg->epsrel,
                            & iter, hfun, hdata, & stop_request);

        chi = gsl_blas_dnrm2(s->f);
        result->chisq = 1.0E6 * pow(chi, 2.0) / f->n;
        result->status = status;
        result->iter = iter;
        if (result->covar) {
            gsl_multifit_covar(s->J, 1e-6, result->covar);
        }
    }

    if(preserve_init_stack) {
        /* we restore the initial stack */
        stack_free(fit->stack);
        fit->stack = initial_stack;
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


int
lmfit_grid(struct fit_engine *fit, struct seeds *seeds,
           double * chisq, str_ptr analysis,
           str_ptr error_msg, int preserve_init_stack,
           gui_hook_func_t hfun, void *hdata)
{
    struct fit_result result[1];
    int want_covar_mat = (analysis ? 1 : 0);
    int status;

    fit_result_init(result, fit, want_covar_mat);
    status = lmfit_grid_run(fit, seeds, preserve_init_stack, result, hfun, hdata);
    if (analysis) {
        fit_result_report(result, analysis, error_msg);
    }
    *chisq = result->chisq;
    fit_result_free(result);
    return status;
}
