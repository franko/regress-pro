#include <assert.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_blas.h>
#include <pthread.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "lmfit.h"
#include "grid-search.h"
#include "stack.h"
#include "fit_result.h"

static void *thr_eval_func(void *arg);

enum {
    SEED_PREPARE_AVAIL,
    SEED_PREPARE_WAIT,
    SEED_PREPARE_END
};

struct thr_eval_data {
    struct fit_engine *fit;

    int seed_status;
    gsl_vector *x_seed;
    pthread_mutex_t seed_lock[1];
    pthread_cond_t seed_cond[1];

    double chisq_best;
    gsl_vector *x_best;
    pthread_mutex_t result_lock[1];
};

static int get_thread_core_number() {
#ifdef WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

void *thr_eval_func(void *arg) {
    struct thr_eval_data *data = (struct thr_eval_data *) arg;
    struct fit_engine *fit = fit_engine_clone(data->fit);

    fit_engine_prepare(fit, data->fit->run->spectr, FIT_ENGINE_KEEP_ACQ);

    gsl_multifit_function_fdf *f = &fit->run->mffun;
    gsl_vector *x = gsl_vector_alloc(f->p);

    /* We choose Levenberg-Marquardt algorithm, scaled version*/
    const gsl_multifit_fdfsolver_type *T = gsl_multifit_fdfsolver_lmsder;
    gsl_multifit_fdfsolver *s = gsl_multifit_fdfsolver_alloc(T, f->n, f->p);

    while (1) {
        pthread_mutex_lock(data->seed_lock);
        while (data->seed_status == SEED_PREPARE_WAIT) {
            pthread_cond_wait(data->seed_cond, data->seed_lock);
        }
        if (data->seed_status == SEED_PREPARE_END) {
            pthread_mutex_unlock(data->seed_lock);
            break;
        }
        gsl_vector_memcpy(x, data->x_seed);
        data->seed_status = SEED_PREPARE_WAIT;
        pthread_mutex_unlock(data->seed_lock);

        fprintf(stderr, "thread: %p seed:", fit);
        for (int k = 0; k < (int) x->size; k++) {
            fprintf(stderr, " %g", gsl_vector_get(x, k));
        }
        fprintf(stderr, "\n");
        fflush(stderr);

        const int search_max_iters = 3;

        gsl_multifit_fdfsolver_set(s, f, x);

        for(int j = 0; j < search_max_iters; j++) {
            int status = gsl_multifit_fdfsolver_iterate(s);
            if(status != 0) {
                break;
            }
        }

        const double chi = gsl_blas_dnrm2(s->f);
        const double chisq = 1.0E6 * pow(chi, 2.0) / f->n;

        pthread_mutex_lock(data->result_lock);
        fprintf(stderr, "thread: %p chisq: %g\n", fit, chisq);
        fflush(stderr);
        if (data->chisq_best < 0 || chisq < data->chisq_best) {
            fprintf(stderr, "thread: %p got best result: %g\n", fit, chisq);
            fflush(stderr);
            data->chisq_best = chisq;
            gsl_vector_memcpy(data->x_best, x);
        }
        pthread_mutex_unlock(data->result_lock);
    }

    fprintf(stderr, "thread: %p terminate\n", fit);
    fflush(stderr);

    gsl_vector_free(x);
    gsl_multifit_fdfsolver_free(s);
    fit_engine_disable(fit);
    fit_engine_free(fit);

    pthread_exit(NULL);
    return NULL;
}

int
lmfit_grid_run(struct fit_engine *fit, struct seeds *seeds,
    int preserve_init_stack, struct fit_result *result,
    gui_hook_func_t hfun, void *hdata)
{
    struct fit_config *cfg = fit->config;
    int nb, iter, nb_grid_pts, j_grid_pts;
    gsl_vector *x, *x_best;
    double *xarr;
    int status, stop_request = 0;
    stack_t *initial_stack;
    seed_t *vseed;

    assert(fit->run);

    vseed = seeds->values;
    nb    = fit->parameters->number;

    assert(fit->parameters->number == seeds->number);

    x     = gsl_vector_alloc(nb);
    x_best = gsl_vector_alloc(nb);

    if(preserve_init_stack) {
        initial_stack = stack_copy(fit->stack);
    }

    xarr = x->data;

    gsl_vector *pstep = gsl_vector_alloc(nb);
    for(int j = 0; j < nb; j++) {
        xarr[j] = fit_engine_get_seed_value(fit, &fit->parameters->values[j], &vseed[j]);
        gsl_vector_set(x_best, j, xarr[j]);
        if(vseed[j].type == SEED_RANGE) {
            xarr[j] -= vseed[j].delta;
        }
    }

    for(int j = 0; j < nb; j++) {
        if(vseed[j].type != SEED_RANGE) continue;
        fit_param_t fp = fit->parameters->values[j];
        double delta = vseed[j].delta;
        double es = fit_engine_estimate_param_grid_step(fit, x_best, &fp, delta);
        gsl_vector_set(pstep, j, es);
    }

    nb_grid_pts = 1;
    for(int j = nb-1; j >= 0; j--) {
        if(vseed[j].type == SEED_RANGE) {
            seed_t *cs = &vseed[j];
            nb_grid_pts *= 2 * cs->delta / gsl_vector_get(pstep, j) + 1;
        }
    }

    if(hfun) {
        (*hfun)(hdata, 0.0, "Running grid search...");
    }

    struct thr_eval_data thr_data = {
        fit,
        SEED_PREPARE_WAIT, x, {PTHREAD_MUTEX_INITIALIZER}, {PTHREAD_COND_INITIALIZER},
        -1.0, x_best, {PTHREAD_MUTEX_INITIALIZER}
    };

    const int threads_number = get_thread_core_number();
    fprintf(stderr, "thread number %d\n", threads_number);
    pthread_t thr[threads_number];
    for (int i = 0; i < threads_number; i++) {
        pthread_create(&thr[i], NULL, thr_eval_func, &thr_data);
    }

    result->interrupted = 0;
    result->chisq_threshold = cfg->chisq_threshold;
    for(j_grid_pts = 0; ; j_grid_pts++) {
        while (1) {
            pthread_mutex_lock(thr_data.seed_lock);
            if (thr_data.seed_status == SEED_PREPARE_WAIT) {
                thr_data.seed_status = SEED_PREPARE_AVAIL;
                gsl_vector_memcpy(thr_data.x_seed, x);
                pthread_mutex_unlock(thr_data.seed_lock);
                pthread_cond_signal(thr_data.seed_cond);
                break;
            } else {
                pthread_mutex_unlock(thr_data.seed_lock);
            }
        }

        if(hfun) {
            float xf = j_grid_pts / (float)nb_grid_pts;
            stop_request = (*hfun)(hdata, xf, NULL);
            if(stop_request) {
                break;
            }
        }

        int j;
        for(j = nb-1; j >= 0; j--) {
            if(vseed[j].type == SEED_RANGE) {
                xarr[j] += gsl_vector_get(pstep, j);
                if(xarr[j] > vseed[j].seed + vseed[j].delta) {
                    xarr[j] = vseed[j].seed - vseed[j].delta;
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
//    if(j < 0 || stop_request) {
        gsl_vector_memcpy(x, thr_data.x_best);
        const double chisq = thr_data.chisq_best;
//    }

    while (1) {
        pthread_mutex_lock(thr_data.seed_lock);
        if (thr_data.seed_status == SEED_PREPARE_WAIT) {
            thr_data.seed_status = SEED_PREPARE_END;
            pthread_mutex_unlock(thr_data.seed_lock);
            pthread_cond_signal(thr_data.seed_cond);
            break;
        } else {
            pthread_mutex_unlock(thr_data.seed_lock);
        }
    }

    for (int i = 0; i < threads_number; i++) {
        pthread_join(thr[i], NULL);
    }

    result->gsearch_chisq = chisq;
    result->chisq = chisq;
    gsl_vector_memcpy(result->gsearch_x, x);
    result->interrupted = stop_request;

    if(stop_request == 0) {
        gsl_multifit_function_fdf *f = &fit->run->mffun;
        const gsl_multifit_fdfsolver_type *T = gsl_multifit_fdfsolver_lmsder;
        gsl_multifit_fdfsolver *s = gsl_multifit_fdfsolver_alloc(T, f->n, f->p);

        status = lmfit_iter(x, f, s, cfg->nb_max_iters,
            cfg->epsabs, cfg->epsrel,
            &iter, hfun, hdata, &stop_request);

        const double chi = gsl_blas_dnrm2(s->f);
        result->chisq = 1.0E6 * pow(chi, 2.0) / f->n;
        result->status = status;
        result->iter = iter;

        gsl_multifit_fdfsolver_free(s);
    }

    if(preserve_init_stack) {
        /* we restore the initial stack */
        stack_free(fit->stack);
        fit->stack = initial_stack;
    } else {
        /* we take care to commit the results obtained from the fit */
        fit_engine_commit_parameters(fit, x);
        fit_engine_update_disp_info(fit);
    }

    gsl_vector_memcpy(fit->run->results, x);

    gsl_vector_free(x);
    gsl_vector_free(x_best);
    gsl_vector_free(pstep);

    return status;
}

int
lmfit_grid(struct fit_engine *fit, struct seeds *seeds,
           struct lmfit_result *result, str_ptr analysis, int preserve_init_stack,
           gui_hook_func_t hfun, void *hdata)
{
    struct fit_result grid_result[1];
    int status;

    fit_result_init(grid_result, fit);
    status = lmfit_grid_run(fit, seeds, preserve_init_stack, grid_result, hfun, hdata);
    if (analysis) {
        fit_result_report(grid_result, analysis);
    }
    result->chisq = grid_result->chisq;
    result->nb_points = spectra_points(fit->run->spectr);
    result->nb_iterations = grid_result->iter;
    result->gsl_status = (grid_result->interrupted ? LMFIT_USER_INTERRUPTED : grid_result->status);
    fit_result_free(grid_result);
    return status;
}
