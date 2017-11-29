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

struct thr_shared_data {
    struct fit_engine *fit;
    pthread_mutex_t lock[1];
    float chisq_best;
    float chisq_threshold;
    gsl_vector *x_best;
    int seed_counter;
    short int solution_found;
    short int stop_request;

    const int dim;
    int *modulo;
    double *x0, *dx;
};

struct thr_eval_data {
    struct thr_shared_data *shared;
    int start_index;
    int index_count;
};

#ifdef WIN32
static int get_thread_core_number() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

static inline void msleep(int ms) {
    Sleep(ms);
}
#else
static int get_thread_core_number() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

static inline void msleep(int ms) {
    usleep(ms * 1000);
}
#endif

void grid_point_set(int dim, const int modulo[], const double x0[], const double dx[], const int index, double x[]) {
    int q = index;
    for (int k = dim - 1; k >= 0; k--) {
        const int r = q % modulo[k];
        q = (q - r) / modulo[k];
        x[k] = x0[k] + r * dx[k];
    }
}

void grid_point_increment(int dim, const int modulo[], const double x0[], const double dx[], double x[]) {
    for (int k = dim - 1; k >= 0; k--) {
        x[k] += dx[k];
        if (x[k] > x0[k] + modulo[k] * dx[k] - dx[k] / 2) {
            x[k] = x0[k];
            continue;
        }
        break;
    }
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
    struct thr_shared_data *shared = data->shared;
    struct fit_engine *fit = fit_engine_clone(shared->fit);

    fit_engine_prepare(fit, shared->fit->run->spectr, FIT_KEEP_ACQUISITION|FIT_ENABLE_SUBSAMPLING);

    gsl_multifit_function_fdf *f = &fit->run->mffun;
    gsl_vector *x = gsl_vector_alloc(f->p);

    /* We choose Levenberg-Marquardt algorithm, scaled version*/
    const gsl_multifit_fdfsolver_type *T = gsl_multifit_fdfsolver_lmsder;
    gsl_multifit_fdfsolver *s = gsl_multifit_fdfsolver_alloc(T, f->n, f->p);

    grid_point_set(shared->dim, shared->modulo, shared->x0, shared->dx, data->start_index, x->data);

    for (int index_count = 0; index_count < data->index_count; index_count++) {
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

        pthread_mutex_lock(shared->lock);
        shared->seed_counter++;
        if (shared->chisq_best < 0 || chisq < shared->chisq_best) {
            shared->chisq_best = chisq;
            gsl_vector_memcpy(shared->x_best, x);
            if (chisq < shared->chisq_threshold) {
                shared->solution_found = 1;
            }
        }
        if (shared->solution_found || shared->stop_request) {
            pthread_mutex_unlock(shared->lock);
            break;
        }
        pthread_mutex_unlock(shared->lock);

        grid_point_increment(shared->dim, shared->modulo, shared->x0, shared->dx, x->data);
    }

    gsl_vector_free(x);
    gsl_multifit_fdfsolver_free(s);
    fit_engine_disable(fit);
    fit_engine_free(fit);

    pthread_exit(nullptr);
    return nullptr;
}

static void
lmfit_grid_run(fit_engine *fit, seeds *seeds, gsl::vector& x, int preserve_init_stack, fit_result *result,
    lmfit_result *lmresult, gui_hook_func_t hfun, void *hdata)
{
    struct fit_config *cfg = fit->config;
    int stop_request = 0;
    stack_t *initial_stack = nullptr;
    const int dim = fit->parameters->number;
    const seed_t *vseed = seeds->values;

    assert(fit->parameters->number == seeds->number);

    if(preserve_init_stack) {
        initial_stack = stack_copy(fit->stack);
    }

    /* We store in x the central coordinates of the grid. */
    for(int j = 0; j < dim; j++) {
        x[j] = fit_engine_get_seed_value(fit, &fit->parameters->values[j], &vseed[j]);
    }

    int modulo[dim];
    double x0[dim], dx[dim];

    for(int j = 0; j < dim; j++) {
        const double x_center = x[j];
        if(vseed[j].type == SEED_RANGE) {
            const fit_param_t fp = fit->parameters->values[j];
            const double delta = vseed[j].delta;
            x0[j] = x_center - delta;
            dx[j] = fit_engine_estimate_param_grid_step(fit, x, &fp, delta);
            modulo[j] = (int) (2 * delta / dx[j]) + 1;
        } else {
            x0[j] = x_center;
            dx[j] = 1.0;
            modulo[j] = 1;
        }
    }

    if(hfun) {
        (*hfun)(hdata, 0.0, "Running grid search...");
    }

    struct thr_shared_data shared_data = {
        fit,
        {PTHREAD_MUTEX_INITIALIZER},
        -1.0, float(cfg->chisq_threshold), x,
        0, 0, 0,
        dim, modulo, x0, dx
    };

    const int nb_grid_pts = grid_point_count(dim, modulo, x0, dx);
    const int threads_number = get_thread_core_number();
    pthread_t thr[threads_number];
    struct thr_eval_data thr_data[threads_number];
    int index = 0;
    const int index_range = nb_grid_pts / threads_number;
    const int rem = nb_grid_pts % threads_number;
    for (int i = 0; i < threads_number; i++, index += index_range) {
        thr_data[i].shared = &shared_data;
        thr_data[i].start_index = index;
        thr_data[i].index_count = (i < rem ? index_range + 1 : index_range);
        pthread_create(&thr[i], nullptr, thr_eval_func, &thr_data[i]);
    }

    result->interrupted = 0;
    result->chisq_threshold = cfg->chisq_threshold;

    while (!shared_data.stop_request && !shared_data.solution_found) {
        msleep(50);
        pthread_mutex_lock(shared_data.lock);
        if (shared_data.seed_counter >= nb_grid_pts) {
            pthread_mutex_unlock(shared_data.lock);
            break;
        }
        if(hfun) {
            float xf = shared_data.seed_counter / (float)nb_grid_pts;
            shared_data.stop_request = (*hfun)(hdata, xf, nullptr);
        }
        pthread_mutex_unlock(shared_data.lock);
    }

    for (int i = 0; i < threads_number; i++) {
        pthread_join(thr[i], nullptr);
    }

    const double chisq = shared_data.chisq_best;

    result->gsearch_chisq = chisq;
    result->chisq = chisq;
    gsl_vector_memcpy(result->gsearch_x, x);
    result->interrupted = stop_request;

    if(stop_request == 0) {
        fit_engine_lmfit(fit, x, lmresult, cfg, hfun, hdata, stop_request);
    } else {
        lmfit_result_error_init(lmresult, LMFIT_USER_INTERRUPTED);
    }

    if (preserve_init_stack) {
        /* we restore the initial stack */
        stack_free(fit->stack);
        fit->stack = initial_stack;
    } else {
        /* we take care to commit the results obtained from the fit */
        fit_engine_commit_fit_results(fit, x);
    }
}

void
lmfit_grid(fit_engine *fit, spectrum *spectrum, gsl::vector& x, seeds *seeds, lmfit_result *result, str_ptr analysis, int preserve_init_stack,
           gui_hook_func_t hfun, void *hdata)
{
    struct fit_result grid_result[1];

    fit_engine_prepare(fit, spectrum, FIT_RESET_ACQUISITION);
    fit_result_init(grid_result, fit);
    lmfit_grid_run(fit, seeds, x, preserve_init_stack, grid_result, result, hfun, hdata);
    if (analysis) {
        fit_result_report(grid_result, analysis);
    }
    fit_result_free(grid_result);
    fit_engine_disable(fit);
}
