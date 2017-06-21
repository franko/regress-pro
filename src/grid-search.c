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

    gsl_vector *x_seed;
    pthread_mutex_t seed_lock[1];
    pthread_cond_t seed_cond[1];

    float chisq_best;
    float chisq_threshold;
    gsl_vector *x_best;
    pthread_mutex_t result_lock[1];

    short int seed_status;
    short int solution_found;
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

void grid_point_set(int dim, const int modulo[], const double x0[], const double dx[], const int index, double x[]) {
    int q = index;
    for (int k = dim - 1; k >= 0; k--) {
        int r = q % modulo[k];
        q = (q - r) / modulo[k];
        x[k] = x0[k] + r * dx[k];
    }
}

/* Return non-zero if there is an overflow. */
int grid_point_increment(int dim, const int modulo[], const double x0[], const double dx[], double x[]) {
    for (int k = dim - 1; k >= 0; k--) {
        x[k] += dx[k];
        if (x[k] > x0[k] + modulo[k] * dx[k] - dx[k] / 2) {
            x[k] = x0[k];
            continue;
        }
        return 0;
    }
    return 1;
}

int grid_point_count(int dim, const int modulo[], const double x0[], const double dx[]) {
    int n = modulo[0];
    for (int k = 1; k < dim; k++) {
        n *= modulo[k];
    }
    return n;
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
        if (data->seed_status == SEED_PREPARE_END) {
            fprintf(stderr, "thread: %p abandon\n", fit);
            fflush(stderr);
            pthread_mutex_unlock(data->seed_lock);
            break;
        }
        while (data->seed_status == SEED_PREPARE_WAIT) {
            pthread_cond_wait(data->seed_cond, data->seed_lock);
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
        int solution_found = 0;
        if (data->chisq_best < 0 || chisq < data->chisq_best) {
            fprintf(stderr, "thread: %p got best result: %g\n", fit, chisq);
            fflush(stderr);
            data->chisq_best = chisq;
            gsl_vector_memcpy(data->x_best, x);
            if (chisq < data->chisq_threshold) {
                solution_found = 1;
            }
        }
        pthread_mutex_unlock(data->result_lock);

        if (solution_found) {
            fprintf(stderr, "thread: %p found solution\n", fit);
            fflush(stderr);
            data->solution_found = 1;
            pthread_mutex_lock(data->seed_lock);
            data->seed_status = SEED_PREPARE_END;
            pthread_mutex_unlock(data->seed_lock);
            break;
        }
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
    int iter, nb_grid_pts, j_grid_pts;
    gsl_vector *x, *x_best;
    int status, stop_request = 0;
    stack_t *initial_stack;
    const int dim = fit->parameters->number;
    const seed_t *vseed = seeds->values;

    assert(fit->run);
    assert(fit->parameters->number == seeds->number);

    x      = gsl_vector_alloc(dim);
    x_best = gsl_vector_alloc(dim);

    if(preserve_init_stack) {
        initial_stack = stack_copy(fit->stack);
    }

    /* We store in x_best the central coordinates of the grid. */
    for(int j = 0; j < dim; j++) {
        const double xc = fit_engine_get_seed_value(fit, &fit->parameters->values[j], &vseed[j]);
        gsl_vector_set(x_best, j, xc);
    }

    int modulo[dim];
    double x0[dim], dx[dim];

    for(int j = 0; j < dim; j++) {
        const double x_center = gsl_vector_get(x_best, j);
        if(vseed[j].type == SEED_RANGE) {
            const fit_param_t fp = fit->parameters->values[j];
            const double delta = vseed[j].delta;
            x0[j] = x_center - delta;
            dx[j] = fit_engine_estimate_param_grid_step(fit, x_best, &fp, delta);
            modulo[j] = (int) (2 * delta / dx[j]) + 1;
        } else {
            x0[j] = x_center;
            dx[j] = 1.0;
            modulo[j] = 1;
        }
    }

    grid_point_set(dim, modulo, x0, dx, 0, x->data);

    nb_grid_pts = grid_point_count(dim, modulo, x0, dx);

    if(hfun) {
        (*hfun)(hdata, 0.0, "Running grid search...");
    }

    struct thr_eval_data thr_data = {
        fit,
        x, {PTHREAD_MUTEX_INITIALIZER}, {PTHREAD_COND_INITIALIZER},
        -1.0, cfg->chisq_threshold, x_best, {PTHREAD_MUTEX_INITIALIZER},
        SEED_PREPARE_WAIT, 0
    };

    const int threads_number = get_thread_core_number();
    fprintf(stderr, "thread number %d\n", threads_number);
    pthread_t thr[threads_number];
    for (int i = 0; i < threads_number; i++) {
        pthread_create(&thr[i], NULL, thr_eval_func, &thr_data);
    }

    result->interrupted = 0;
    result->chisq_threshold = cfg->chisq_threshold;
    int solution_found = 0;
    for(j_grid_pts = 0; ; j_grid_pts++) {
        while (1) {
            pthread_mutex_lock(thr_data.seed_lock);
            if (thr_data.seed_status == SEED_PREPARE_END) {
                solution_found = 1;
                pthread_mutex_unlock(thr_data.seed_lock);
                break;
            }
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

        if (solution_found) {
            break;
        }

        if(hfun) {
            float xf = j_grid_pts / (float)nb_grid_pts;
            stop_request = (*hfun)(hdata, xf, NULL);
            if(stop_request) {
                break;
            }
        }

        if (grid_point_increment(dim, modulo, x0, dx, x->data)) {
            break;
        }
    }

    if (!solution_found) {
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
    }

    gsl_vector_memcpy(x, thr_data.x_best);
    const double chisq = thr_data.chisq_best;

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
