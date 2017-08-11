
/* fit-engine.c
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

#include <assert.h>
#include <string.h>
#include "fit-engine.h"
#include "refl-fit.h"
#include "refl-kernel.h"
#include "elliss-fit.h"
#include "elliss.h"
#include "error-messages.h"
#include "minsampling.h"
#include "fit-timestamp.h"

static void build_fit_engine_cache(struct fit_engine *f);

static void dispose_fit_engine_cache(struct fit_engine *f);

void
build_stack_cache(struct stack_cache *cache, stack_t *stack,
                  struct spectrum *spectr, int th_only_optimize, int require_acquisition_jacob)
{
    size_t nb_med = stack->nb;

    cache->nb_med = nb_med;

#if 0
    cache->deriv_info = emalloc(nb_med * sizeof(struct deriv_info));

    for(size_t j = 0; j < nb_med; j++) {
        struct deriv_info *di = cache->deriv_info + j;
        size_t tpnb = disp_get_number_of_params(stack->disp[j]);

        di->is_valid = 0;
        di->val = (tpnb == 0 ? NULL : cmpl_vector_alloc(tpnb));
    }
#endif

    cache->th_only = th_only_optimize;
    cache->require_acquisition_jacob = require_acquisition_jacob;

    if(th_only_optimize) {
        int k, npt = spectra_points(spectr);
        cmpl *ns;

        cache->ns_full_spectr = emalloc(nb_med * npt * sizeof(cmpl));

        for(k = 0, ns = cache->ns_full_spectr; k < npt; ns += nb_med, k++) {
            double lambda = get_lambda_by_index(spectr, k);
            stack_get_ns_list(stack, ns, lambda);
        }
    } else {
        cache->ns_full_spectr = NULL;
    }

    cache->is_valid = 1;
}

void
dispose_stack_cache(struct stack_cache *cache)
{
    if(! cache->is_valid) {
        return;
    }
#if 0
    int nb_med = cache->nb_med;
    for(int j = 0; j < nb_med; j++) {
        struct deriv_info *di = & cache->deriv_info[j];
        if(di->val) {
            cmpl_vector_free(di->val);
        }
    }
    free(cache->deriv_info);
#endif

    if(cache->ns_full_spectr) {
        free(cache->ns_full_spectr);
    }

    cache->is_valid = 0;
}

// ********************** DONE
// To be reviewed later. Build cache structure only for first spectrum.
void
build_fit_engine_cache(struct fit_engine *f)
{
    assert(f->spectra_number >= 1);
    int RI_fixed = (f->spectra_number == 1) && fit_parameters_are_RI_fixed(f->parameters);
    int require_acquisition_jacob = fit_parameters_contains_acquisition_parameters(f->parameters);
    build_stack_cache(&f->cache, f->stack_list[0], f->spectra_list[0].spectrum, RI_fixed, require_acquisition_jacob);
}

void
dispose_fit_engine_cache(struct fit_engine *f)
{
    dispose_stack_cache(&f->cache);
}

// ************************** DONE
int
fit_engine_apply_param(struct fit_engine *fit, const fit_param_t *fp, double val)
{
    const int fp_acquisition = scope_acquisition_raw(fp->scope);
    if (fp_acquisition >= 0) {
        for (int i = 0; i < fit->spectra_number; i++) {
            struct spectrum_item *sitem = &fit->spectra_list[i];
            if (scope_is_inside(sitem->scope, fp->scope)) {
                int res = acquisition_apply_param(sitem->acquisition, fp->id, val);
                if (res != 0) return res;
            }
        }
    } else {
        for (int k = 0; k < fit->samples_number; k++) {
            const int sample_scope = FIT_PARAM_SCOPE(fit->spectra_groups[k] + 1, k + 1, 0);
            if (scope_is_inside(sample_scope, fp->scope)) {
                int res = stack_apply_param(fit->stack_list[k], fp, val);
                if (res != 0) return res;
            }
        }
    }
    return 0;
}

void
fit_engine_apply_parameters(struct fit_engine *fit,
                            const struct fit_parameters *fps,
                            const gsl_vector *x)
{
    size_t j;

    for(j = 0; j < fps->number; j++) {
        const fit_param_t *fp = fps->values + j;
        const double pval = gsl_vector_get(x, j);
        int status = fit_engine_apply_param(fit, fp, pval);
        /* No error should never occurs here because the fit parameters
        are checked in advance. */
        assert(status == 0);
    }
}

void
fit_engine_commit_parameters(struct fit_engine *fit, const gsl_vector *x)
{
    struct fit_parameters const * fps = fit->parameters;
    fit_engine_apply_parameters(fit, fps, x);
}

static void
spectrum_clip(const struct spectral_range *range, double *wlmin, double *wlmax)
{
    if (range->active) {
        if (range->min > *wlmin) {
            *wlmin = range->min;
        }
        if (range->max < *wlmax) {
            *wlmax = range->max;
        }
    }
}

static void
spectrum_limits(const struct spectral_range *range, const struct spectrum *s, double *wlmin, double *wlmax)
{
    spectra_wavelength_range(s, wlmin, wlmax);
    spectrum_clip(range, wlmin, wlmax);
}

// ************************ DONE
// Set spectra limits to the largest limits of all spectra.
void
fit_engine_get_wavelength_limits(const struct fit_engine *fit, double *wlmin, double *wlmax)
{
    for (int i = 0; i < fit->spectra_number; i++) {
        const struct spectrum_item *sitem = &fit->spectra_list[i];
        double wlmin_i, wlmax_i;
        spectrum_limits(&fit->config->spectr_range, sitem->spectrum, &wlmin_i, &wlmax_i);
        if (i == 0 || wlmin_i < *wlmin) *wlmin = wlmin_i;
        if (i == 0 || wlmax_i > *wlmax) *wlmax = wlmax_i;
    }
}

void
fit_engine_update_disp_info(struct fit_engine *fit)
{
    double wavelength_start, wavelength_end;
    fit_engine_get_wavelength_limits(fit, &wavelength_start, &wavelength_end);
    for (int k = 0; k < fit->samples_number; k++) {
        stack_t *stack = fit->stack_list[k];
        for (int i = 0; i < stack->nb; i++) {
            disp_t *d = stack->disp[i];
            for(size_t j = 0; j < fit->parameters->number; j++) {
                const fit_param_t *fp = &fit->parameters->values[j];
                if (fp->id == PID_LAYER_N && fp->layer_nb == i) {
                    disp_set_info_wavelength(d, wavelength_start, wavelength_end);
                    char buffer[128];
                    if (fit_timestamp(128, buffer) != 0) {
                        disp_set_modifications_flag(d, buffer);
                    }
                    break;
                }
            }
        }
    }
}

// **************************** DONE
// Temporary solution. This function is only used by interactive fit.
void
fit_engine_copy_disp_info(struct fit_engine *dst, const struct fit_engine *src)
{
    assert(src->samples_number == dst->samples_number);
    for (int k = 0; k < src->samples_number; k++) {
        stack_t *dst_stack = dst->stack_list[k];
        stack_t *src_stack = src->stack_list[k];
        for (int i = 0; i < dst_stack->nb; i++) {
            disp_info_copy(dst_stack->disp[i]->info, src_stack->disp[i]->info);
        }
    }
}

#ifdef DEBUG


struct fdf_deriv_ws {
    gsl_vector *fmh, *fph;
    gsl_vector *fm1, *fp1;
    gsl_vector *r3, *r5, *r_aux;
};

static void
central_deriv(const gsl_multifit_function_fdf *f, gsl_vector *x, int parameter_index, double h,
              struct fdf_deriv_ws *workspace, gsl_vector *result, gsl_vector *abserr)
{
  /* Compute the derivative using the 5-point rule (x-h, x-h/2, x,
     x+h/2, x+h). Note that the central point is not used.

     Compute the error using the difference between the 5-point and
     the 3-point rule (x-h,x,x+h). Again the central point is not
     used. */

    const double x0 = gsl_vector_get(x, parameter_index);

    gsl_vector_set(x, parameter_index, x0 - h);
    f->f(x, f->params, workspace->fm1);

    gsl_vector_set(x, parameter_index, x0 + h);
    f->f(x, f->params, workspace->fp1);

    gsl_vector_set(x, parameter_index, x0 - h / 2);
    f->f(x, f->params, workspace->fmh);

    gsl_vector_set(x, parameter_index, x0 + h / 2);
    f->f(x, f->params, workspace->fph);

    // Reset array element to its original value.
    gsl_vector_set(x, parameter_index, x0);

    gsl_vector_memcpy(workspace->r3, workspace->fp1);
    gsl_vector_sub(workspace->r3, workspace->fm1);
    gsl_vector_scale(workspace->r3, 0.5);

    // double r3 = 0.5 * (fp1 - fm1);

    gsl_vector_memcpy(workspace->r5, workspace->fph);
    gsl_vector_sub(workspace->r5, workspace->fmh);
    gsl_vector_scale(workspace->r5, 4.0 / 3.0);

    gsl_vector_memcpy(workspace->r_aux, workspace->r3);
    gsl_vector_scale(workspace->r_aux, 1.0 / 3.0);
    gsl_vector_sub(workspace->r5, workspace->r_aux);

    // double r5 = (4.0 / 3.0) * (fph - fmh) - (1.0 / 3.0) * r3;

    gsl_vector_memcpy(result, workspace->r5);
    gsl_vector_scale(result, 1.0 / h);

    gsl_vector_memcpy(abserr, workspace->r5);
    gsl_vector_sub(abserr, workspace->r3);
    gsl_vector_scale(abserr, 1.0 / h);

    for (int i = 0; i < (int)abserr->size; i++) {
        gsl_vector_set(abserr, i, fabs(gsl_vector_get(abserr, i)));
    }

    // double e3 = (fabs (fp1) + fabs (fm1)) * GSL_DBL_EPSILON;
    // double e5 = 2.0 * (fabs (fph) + fabs (fmh)) * GSL_DBL_EPSILON + e3;

  /* The next term is due to finite precision in x+h = O (eps * x) */

    // double dy = GSL_MAX (fabs (r3 / h), fabs (r5 / h)) *(fabs (x) / h) * GSL_DBL_EPSILON;

  /* The truncation error in the r5 approximation itself is O(h^4).
     However, for safety, we estimate the error from r5-r3, which is
     O(h^2).  By scaling h we will minimise this estimated error, not
     the actual truncation error in r5. */

  // *result = r5 / h;
  // *abserr_trunc = fabs ((r5 - r3) / h); /* Estimated truncation error O(h^2) */
  // *abserr_round = fabs (e5 / h) + dy;   /* Rounding error (cancellations) */
}

static void
fit_engine_check_deriv(struct fit_engine *fit) {
    const int points_number = fit->mffun.n;
    const int parameters_number = fit->mffun.p;
    struct fdf_deriv_ws ws[1];

    fprintf(stderr, "Checking derivatives for fit.\nnumber of points: %d, number of parameters: %d\n", points_number, parameters_number);

    ws->fm1 = gsl_vector_alloc(points_number);
    ws->fp1 = gsl_vector_alloc(points_number);
    ws->fmh = gsl_vector_alloc(points_number);
    ws->fph = gsl_vector_alloc(points_number);
    ws->r3  = gsl_vector_alloc(points_number);
    ws->r5  = gsl_vector_alloc(points_number);
    ws->r_aux = gsl_vector_alloc(points_number);

    gsl_matrix *num_jacob = gsl_matrix_alloc(points_number, parameters_number);
    gsl_matrix *abserr_mat = gsl_matrix_alloc(points_number, parameters_number);
    gsl_vector *x = gsl_vector_alloc(parameters_number);

    for(int j = 0; j < parameters_number; j++) {
        double seed = fit_engine_get_parameter_value(fit, &fit->parameters->values[j]);
        gsl_vector_set(x, j, seed);
    }

    for(int j = 0; j < parameters_number; j++) {
        const double seed = gsl_vector_get(x, j);
        double delta = fabs(seed) / 100.0;
        if (delta < 1e-8) delta = 1e-8;

        gsl_vector_view jacob_row_view = gsl_matrix_column(num_jacob, j);
        gsl_vector *df = &jacob_row_view.vector;

        gsl_vector_view abserr_view = gsl_matrix_column(abserr_mat, j);
        gsl_vector *abserr = &abserr_view.vector;

        central_deriv(&fit->mffun, x, j, delta, ws, df, abserr);
    }

    gsl_matrix *com_jacob = gsl_matrix_alloc(points_number, parameters_number);
    fit->mffun.df(x, fit, com_jacob);

    for (int i = 0; i < points_number; i++) {
        for (int j = 0; j < parameters_number; j++) {
            double num = gsl_matrix_get(num_jacob, i, j);
            double abserr = gsl_matrix_get(abserr_mat, i, j);
            double com = gsl_matrix_get(com_jacob, i, j);
            double err_min = fabs(num) * 1e-8;
            if (abserr < err_min) abserr = err_min;
            if (com > num + abserr || com < num - abserr) {
                fprintf(stderr, "DERIV DIFFER %d PARAM: %d: NUM: %g +/- %g, COMPUTED: %g\n", i, j, num, abserr, com);
            }
        }
    }

    gsl_matrix_free(num_jacob);
    gsl_matrix_free(com_jacob);
    gsl_matrix_free(abserr_mat);
    gsl_vector_free(x);
    gsl_vector_free(ws->fmh);
    gsl_vector_free(ws->fph);
    gsl_vector_free(ws->fm1);
    gsl_vector_free(ws->fp1);
    gsl_vector_free(ws->r3);
    gsl_vector_free(ws->r5);
    gsl_vector_free(ws->r_aux);

    fprintf(stderr, "done\n");
    fflush(stderr);
}
#endif

static void
mult_layer_refl_sys(const enum system_kind sys_kind, size_t nb, const cmpl ns[],
                   const double ds[], double lambda,
                   const struct acquisition_parameters *acquisition, double result[],
                   double *jacob_th, cmpl *jacob_n, double *jacob_acq)
{
    switch (sys_kind) {
    case SYSTEM_SR:
    {
        mult_layer_refl_sr(nb, ns, ds, lambda, acquisition, result, jacob_th, jacob_n, jacob_acq);
        break;
    }
    case SYSTEM_SE_RPE:
    case SYSTEM_SE_RAE:
    case SYSTEM_SE:
    {
        const enum se_type se_type = SE_TYPE(sys_kind);
        ell_ab_t e;
        mult_layer_refl_se(se_type, nb, ns, ds, lambda, acquisition, e, jacob_th, jacob_n, jacob_acq);
        result[0] = e->alpha;
        result[1] = e->beta;
        break;
    }
    default:
        break;
    }
}

static inline int se_acquisition_enum(int pid) {
    switch(pid) {
    case PID_AOI:       return SE_AOI;
    case PID_ANALYZER:  return SE_ANALYZER;
    case PID_POLARIZER: return SE_POLARIZER;
    case PID_BANDWIDTH: return SE_BANDWIDTH;
    case PID_NUMAP:     return SE_NUMAP;
    }
    return (-1);
}


static inline int sr_acquisition_enum(int pid) {
    switch(pid) {
    case PID_FIRSTMUL:  return SR_RMULT;
    case PID_BANDWIDTH: return SR_BANDWIDTH;
    }
    return (-1);
}

static inline double select_acquisition_jacob(const enum system_kind sys_kind, const double jacob_acq[], const int channel, const int pid) {
    if (sys_kind == SYSTEM_SE_RAE || sys_kind == SYSTEM_SE_RPE || sys_kind == SYSTEM_SE) {
        const int index = se_acquisition_enum(pid);
        return jacob_acq[SE_ACQ_INDEX(channel, index)];
    } else if (sys_kind == SYSTEM_SR) {
        const int index = sr_acquisition_enum(pid);
        return jacob_acq[index];
    }
    return 0.0;
}

static void
select_param_jacobian(const enum system_kind sys_kind, const int channels_number, const fit_param_t *fp, const stack_t *stack,
                       struct deriv_info *ideriv, double lambda,
                       double result[], double jacob_th[], cmpl jacob_n[], double jacob_acq[])
{
    const int nb_med = stack->nb, nb_lyr = nb_med - 2;
    const int layer = fp->layer_nb;
    double dnr, dni;

    switch(fp->id) {
    case PID_THICKNESS:
        for (int q = 0; q < channels_number; q++) {
            result[q] = jacob_th[q * nb_lyr + (layer - 1)];
        }
        break;
    case PID_LAYER_N:
        get_model_param_deriv(stack->disp[layer], &ideriv[layer], fp, lambda, &dnr, &dni);
        for (int q = 0; q < channels_number; q++) {
            const cmpl drdn = jacob_n[q * nb_med + layer];
            result[q] = creal(drdn) * dnr - cimag(drdn) * dni;
        }
        break;
    case PID_AOI:
    case PID_ANALYZER:
    case PID_NUMAP:
    case PID_BANDWIDTH:
    case PID_FIRSTMUL:
    {
        for (int q = 0; q < channels_number; q++) {
            result[q] = select_acquisition_jacob(sys_kind, jacob_acq, q, fp->id);
        }
        break;
    }
    default:
        memset(result, 0, channels_number * sizeof(double));
    }
}

// ***************************** DONE
// To be improved the deriv_info stuff and optimizations for fixed RIs etc.
static int
fit_fdf(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *jacob)
{
    struct fit_engine *fit = params;
    const int spectra_number = fit->spectra_number;

    /* STEP 1 : We apply the actual values of the fit parameters
                to the stack. */

    fit_engine_commit_parameters(fit, x);

    int spectrum_offset = 0;
    for(int spectrum_no = 0; spectrum_no < spectra_number; spectrum_no++) {
        struct spectrum_item *sitem = &fit->spectra_list[spectrum_no];
        struct spectrum *spectrum = sitem->spectrum;
        const int spectrum_scope = sitem->scope;
        const int sample = scope_sample_raw(spectrum_scope);
        struct stack *stack_sample = fit->stack_list[sample];
        const int nb_med = stack_sample->nb;
        const int npt = spectra_points(spectrum);
        const enum system_kind sys_kind = sitem->acquisition->type;
        const int channels_number = SYSTEM_CHANNELS_NUMBER(sys_kind);
        double jacob_th_data[channels_number * (nb_med - 2)];
        cmpl jacob_n_data[channels_number * nb_med];
        double jacob_acq_data[channels_number * SYSTEM_ACQUISITION_PARAMS_NUMBER(sys_kind)];

        /* Should never fail as spectrum_scope is always fully specified for each
           spectrum_item */
        assert(sample >= 0);

        /* STEP 2 : From the stack we retrive the thicknesses and RIs
        informations. */

        const double *ths = stack_get_ths_list(stack_sample);

        /* TODO: take into account the case of fixed RIs and when acquisition
           parameters are not needed. */
        double *jacob_th  = (jacob ? jacob_th_data  : NULL);
        cmpl *  jacob_n   = (jacob ? jacob_n_data   : NULL);
        double *jacob_acq = (jacob ? jacob_acq_data : NULL);

        for(int j = 0, jpt = spectrum_offset; j < npt; j++, jpt += channels_number) {
            float const * spectr_data = spectra_get_values(spectrum, j);
            const double lambda = spectr_data[0];
            double result[channels_number];

            cmpl ns[nb_med];
            stack_get_ns_list(stack_sample, ns, lambda);

            /* STEP 3 : We call the function to compute the model values. */
            mult_layer_refl_sys(sys_kind, nb_med, ns, ths, lambda, sitem->acquisition, result, jacob_th, jacob_n, jacob_acq);

            if(f != NULL) {
                for (int q = 0; q < channels_number; q++) {
                    gsl_vector_set(f, jpt + q, result[q] - spectr_data[q + 1]);
                }
            }

            if(jacob) {
                struct deriv_info ideriv[nb_med];

                /* TODO: avoid allocating on the heap the ideriv vectors each time.*/
                for(int i = 0; i < nb_med; i++) {
                    ideriv[i].is_valid = 0;
                    const int params_no = disp_get_number_of_params(stack_sample->disp[i]);
                    /* Here params_no can be 0 but is still ok to call cmpl_vector_alloc. */
                    ideriv[i].val = cmpl_vector_alloc(params_no);
                }

                for (int kp = 0; kp < (int) fit->parameters->number; kp++) {
                    const fit_param_t *fp = &fit->parameters->values[kp];
                    if (scope_is_inside(spectrum_scope, fp->scope)) {
                        double fp_jacob[channels_number];
                        select_param_jacobian(sys_kind, channels_number, fp, stack_sample, ideriv, lambda,
                            fp_jacob, jacob_th, jacob_n, jacob_acq);
                        for (int q = 0; q < channels_number; q++) {
                            gsl_matrix_set(jacob, jpt + q, kp, fp_jacob[q]);
                        }
                    } else {
                        for (int q = 0; q < channels_number; q++) {
                            gsl_matrix_set(jacob, jpt + q, kp, 0.0);
                        }
                    }
                }

                for(int i = 0; i < nb_med; i++) {
                    cmpl_vector_free(ideriv[i].val);
                }
            }
        }

        spectrum_offset += channels_number * npt;
    }

    return GSL_SUCCESS;
}

static int
fit_f(const gsl_vector *x, void *params, gsl_vector *f)
{
    return fit_fdf(x, params, f, NULL);
}

static int
fit_df(const gsl_vector *x, void *params, gsl_matrix *jacob)
{
    return fit_fdf(x, params, NULL, jacob);
}

static void
set_spectra_list_capacity(struct fit_engine *f, const int capa) {
    assert(capa >= f->spectra_number);
    free(f->spectra_list);
    f->spectra_capacity = capa * 3 / 2;
    struct spectrum_item *new_list = emalloc(sizeof(struct spectrum_item) * f->spectra_capacity);
    memcpy(new_list, f->spectra_list, sizeof(struct spectrum_item) * f->spectra_number);
    f->spectra_list = new_list;
}

void
fit_engine_clear_spectra(struct fit_engine *f) {
    for (int i = 0; i < f->spectra_number; i++) {
        spectra_free(f->spectra_list[i].spectrum);
    }
    f->spectra_number = 0;
    set_spectra_list_capacity(f, 0);
}

// ************************ DONE
void
fit_engine_add_spectrum(struct fit_engine *f, const struct spectrum *s, const int spectrum_scope)
{
    if (f->spectra_number + 1 > f->spectra_capacity) {
        set_spectra_list_capacity(f, f->spectra_number + 1);
    }

    const int spectrum_no = scope_acquisition_raw(spectrum_scope);
    const int sample = scope_sample_raw(spectrum_scope);
    assert(sample < f->samples_number);

    struct spectrum_item *si = &f->spectra_list[spectrum_no];
    si->spectrum = spectra_copy(s);
    si->acquisition[0] = s->acquisition[0];
    si->scope = spectrum_scope;

    f->spectra_number++;
}

extern int  fit_engine_prepare(struct fit_engine *f, const struct spectrum *s, const int fit_engine_flags)
{
    fit_engine_clear_spectra(f);
    fit_engine_add_spectrum(f, s, FIT_PARAM_SCOPE(1,1,1));
    return fit_engine_prepare_final(f, fit_engine_flags);
}

int
fit_engine_prepare_final(struct fit_engine *fit, const int fit_engine_flags)
{
    const int acq_policy = FIT_OPTIONS_ACQUISITION(fit_engine_flags);
    const int enable_subsampling = FIT_OPTIONS_SUBSAMPLING(fit_engine_flags);
    const struct fit_config *config = fit->config;

    if (acq_policy == FIT_RESET_ACQUISITION) {
        /* Copy all the spectra acquisitions info into the fit structure. */
        for (int j = 0; j < fit->samples_number; j++) {
            fit->acquisitions[j] = fit->spectra_list[j]->acquisition[0];
        }
    }

    int points_number = 0;
    if(config->spectr_range.active) {
        for(int k = 0; k < fit->spectra_number; k++) {
            const float wlmin = config.spectr_range.min, wlmax = config->spectr_range.max;
            struct spectrum *spectrum = fit->spectra_list[k].spectrum;
            spectr_cut_range(spectrum, wlmin, wlmax);
            const int channels_number = SYSTEM_CHANNELS_NUMBER(spectrum->acquisition->type);
            if(config->subsampling && enable_subsampling && channels_number == 2) {
                elliss_sample_minimize(spectrum, 0.05); // FIXME: should work for any number of channels
            }
            points_number += channels_number * spectra_points(spectrum);
        }
    }

    build_fit_engine_cache(fit);

    fit->mffun.f      = &fit_f;
    fit->mffun.df     = &fit_df;
    fit->mffun.fdf    = &fit_fdf;
    fit->mffun.n      = points_number;
    fit->mffun.p      = fit->parameters->number;
    fit->mffun.params = fit;

    if(!cfg->threshold_given) {
        cfg->chisq_threshold = 150; // FIXME
    }

    fit->results = gsl_vector_alloc(fit->parameters->number);

#ifdef DEBUG
    fit_engine_check_deriv(fit);
#endif

    return 0;
}

void
fit_engine_disable(struct fit_engine *fit)
{
    dispose_fit_engine_cache(fit);
    spectra_free(fit->run->spectr);
    gsl_vector_free(fit->run->results);
}

int
check_fit_parameters(struct stack *stack, struct fit_parameters *fps, str_ptr *error_msg)
{
    size_t j, nb_med = (size_t) stack->nb;

    assert(stack->nb >= 2);

    for(j = 0; j < nb_med; j++) {
        if(disp_integrity_check(stack->disp[j])) {
            *error_msg = new_error_message(RECIPE_CHECK, "corrupted material card");
            return 1;
        }
    }

    for(j = 0; j < fps->number; j++) {
        fit_param_t *fp = fps->values + j;

        if (fp->id >= PID_ACQUISITION_PARAMETER && fp->id < PID_INVALID)
            return 0;

        switch(fp->id) {
        case PID_THICKNESS:
            if(fp->layer_nb == 0 || fp->layer_nb >= nb_med - 1) {
                *error_msg = new_error_message(RECIPE_CHECK, "reference to thickness of layer %i", fp->layer_nb);
                return 1;
            }
            break;
        case PID_LAYER_N:
            if(fp->layer_nb >= nb_med) {
                *error_msg = new_error_message(RECIPE_CHECK, "Reference to parameter of material number %i", fp->layer_nb);
                return 1;
            }

            if(disp_check_fit_param(stack->disp[fp->layer_nb], fp)) {
                str_t pname;
                str_init(pname, 15);
                get_param_name(fp, pname);
                *error_msg = new_error_message(RECIPE_CHECK,
                                 "Parameter %s makes no sense for layer %i",
                                 CSTR(pname), fp->layer_nb);
                str_free(pname);
                return 1;
            }
            break;
        default:
            *error_msg = new_error_message(RECIPE_CHECK, "ill-formed fit parameter");
            return 1;
        }
    }

    return 0;
}

// **************** DONE
struct fit_parameters *
fit_engine_get_all_parameters(struct fit_engine *fit) {
    struct fit_parameters *fps = fit_parameters_new();
    for (int i = 0; i < fit->samples_number; i++) {
        /* Set the group to 1 (generic group) and the sample to i + 1 (add 1 because
           0 means undefined sample). */
        const unsigned int sample_scope = FIT_PARAM_SCOPE(1, i + 1, 0);
        const struct stack *stack = fit->stack_list[i];
        stack_get_all_parameters(stack, fps, sample_scope);
    }
    for (int i = 0; i < fit->spectra_number; i++) {
        const struct spectrum_item *sitem = &fit->spectra_list[i];
        acquisition_get_all_parameters(sitem->acquisition, fps, sitem->scope);
    }
    return fps;
}

// **************** DONE
double
fit_engine_get_parameter_value(const struct fit_engine *fit, const fit_param_t *fp)
{
    if(fp->id >= PID_ACQUISITION_PARAMETER) {
        const int spectrum_no = scope_acquisition_any(fp->scope);
        if (spectrum_no < fit->spectra_number) {
            const struct spectrum_item *sitem = &fit->spectra_list[spectrum_no];
            return acquisition_get_parameter(sitem->acquisition, fp->id);
        }
    } else {
        const int sample_no = scope_sample_any(fp->scope);
        if (sample_no < fit->samples_number) {
            const struct stack *stack = fit->stack_list[samples_number];
            return stack_get_parameter_value(stack, fp);
        }
    }
    return 0.0;
}

double
fit_engine_get_seed_value(const struct fit_engine *fit, const fit_param_t *fp, const seed_t *s)
{
    if (s->type == SEED_UNDEF) {
        return fit_engine_get_parameter_value(fit, fp);
    }
    return s->seed;
}

static double
compute_rsquare(const gsl_vector *ref, const gsl_vector *b)
{
    double ssq = 0;
    unsigned int i;
    for (i = 0; i < ref->size; i++) {
        ssq += gsl_vector_get(ref, i) * gsl_vector_get(ref, i);
    }

    double rsq = 0;
    for (i = 0; i < ref->size; i++) {
        double diff = gsl_vector_get(b, i) - gsl_vector_get(ref, i);
        rsq += diff * diff;
    }
    return rsq / ssq;
}

double
fit_engine_estimate_param_grid_step(struct fit_engine *fit, const gsl_vector *x, const fit_param_t *fp, double delta)
{
    int fp_index = fit_parameters_find(fit->parameters, fp);

    gsl_vector *y0 = gsl_vector_alloc(fit->run->mffun.n);
    gsl_vector *y1 = gsl_vector_alloc(fit->run->mffun.n);
    gsl_vector *xtest = gsl_vector_alloc(fit->run->mffun.p);
    gsl_matrix *jacob = gsl_matrix_alloc(fit->run->mffun.n, fit->run->mffun.p);

    gsl_vector_view jview = gsl_matrix_column(jacob, fp_index);
    fit->run->mffun.df(x, fit, jacob);

    gsl_vector_memcpy(xtest, x);
    fit->run->mffun.f(xtest, fit, y0);

    while (1) {
        gsl_vector_set(xtest, fp_index, gsl_vector_get(x, fp_index) + delta);
        fit->run->mffun.f(xtest, fit, y1);

        gsl_vector_sub(y1, y0);
        gsl_vector_scale(y1, 1.0 / delta);

        double r2 = compute_rsquare(&jview.vector, y1);
        if (r2 < 0.2) break;

        delta /= 2;
    }

    gsl_vector_free(y0);
    gsl_vector_free(y1);
    gsl_vector_free(xtest);
    gsl_matrix_free(jacob);
    return delta;
}

void
fit_engine_generate_spectrum(struct fit_engine *fit, struct spectrum *ref,
                             int spectrum_no, struct spectrum *synth)
{
    enum system_kind syskind = ref->acquisition->type;
    const struct spectrum_item *sitem = &fit->spectra_list[spectrum_no];
    const stack_t *stack = fit->stack_list[scope_sample(sitem->scope)];
    const channels_number = SYSTEM_CHANNELS_NUMBER(syskind);
    const size_t nb_med = stack->nb;
    struct data_table *table = synth->table[0].table;
    const int npt = spectra_points(ref);
    cmpl *ns = emalloc(sizeof(cmpl) * nb_med);

    assert(spectra_points(ref) == spectra_points(synth));

    synth->acquisition[0] = ref->acquisition[0];

    const double *ths = stack_get_ths_list(stack);

    for(int j = 0; j < npt; j++) {
        double lambda = get_lambda_by_index(ref, j);

        data_table_set(table, j, 0, lambda);

        stack_get_ns_list(stack, ns, lambda);

        double r[channels_number];
        mult_layer_refl_sys(syskind, nb_med, ns, ths, lambda, sitem->acquisition, r, NULL, NULL, NULL);
        for (int k = 0; k < channels_number; k++) {
            data_table_set(table, j, k + 1, r[k]);
        }
    }

    free(ns);
}

struct fit_engine *
fit_engine_new()
{
    struct fit_engine *fit = emalloc(sizeof(struct fit_engine));
    acquisition_set_default(fit->acquisition);
    fit->parameters = NULL;
    fit->stack = NULL;
    return fit;
}

void
fit_engine_bind(struct fit_engine *fit, const stack_t *stack, const struct fit_config *config, struct fit_parameters *parameters)
{
    fit->config[0] = *config;
    /* fit is not the owner of the "parameters", we just keep a reference */
    fit->parameters = parameters;
    fit->stack = stack_copy(stack);
}

struct fit_engine *
fit_engine_clone(struct fit_engine *src_fit)
{
    struct fit_engine *fit = emalloc(sizeof(struct fit_engine));
    fit->acquisition[0] = src_fit->acquisition[0];
    fit->config[0] = src_fit->config[0];
    fit->parameters = src_fit->parameters;
    if (src_fit->stack) {
        fit->stack = stack_copy(src_fit->stack);
    } else {
        fit->stack = NULL;
    }
    return fit;
}

void
fit_engine_bind_stack(struct fit_engine *fit, stack_t *stack)
{
    if (fit->stack) {
        stack_free(fit->stack);
    }
    fit->stack = stack;
}

stack_t *
fit_engine_yield_stack(struct fit_engine *fit)
{
    stack_t *s = fit->stack;
    fit->stack = NULL;
    return s;
}

void
fit_engine_free(struct fit_engine *fit)
{
    if (fit->stack) {
        stack_free(fit->stack);
    }
    free(fit);
}

void
fit_engine_print_fit_results(struct fit_engine *fit, str_t text, int tabular)
{
    str_t pname, value;
    size_t j;

    str_set_null(text);
    str_init(pname, 15);
    str_init(value, 15);

    for(j = 0; j < fit->parameters->number; j++) {
        fit_param_t *fp = fit->parameters->values + j;
        get_param_name(fp, pname);
        if(tabular) {
            str_printf(value, "%.6g", gsl_vector_get(fit->run->results, j));
            str_pad(value, 12, ' ');
            str_append(text, value, 0);
        } else
            str_printf_add(text, "%9s : %.6g\n", CSTR(pname),
                           gsl_vector_get(fit->run->results, j));
    }

    str_free(value);
    str_free(pname);
}

void
fit_engine_set_acquisition(struct fit_engine *fit, struct acquisition_parameters *acquisition)
{
    fit->acquisition[0] = *acquisition;
}

void
fit_config_set_default(struct fit_config *cfg)
{
    cfg->threshold_given = 0;
    cfg->nb_max_iters = 30;
    cfg->subsampling = 1;
    cfg->spectr_range.active = 0;
    cfg->epsabs = 1.0E-7;
    cfg->epsrel = 1.0E-7;
}

int
fit_config_write(writer_t *w, const struct fit_config *config)
{
    writer_printf(w, "fit-config");
    writer_newline_enter(w);
    if (config->threshold_given) {
        writer_printf(w, "threshold %g", config->chisq_threshold);
        writer_newline(w);
    }
    writer_printf(w, "max-iterations %d", config->nb_max_iters);
    writer_newline(w);

    writer_printf(w, "subsampling %d", config->subsampling);
    writer_newline(w);

    if (config->spectr_range.active) {
        writer_printf(w, "wavelength-range %g %g", config->spectr_range.min, config->spectr_range.max);
        writer_newline(w);
    }

    writer_printf(w, "epsilon %g %g", config->epsabs, config->epsrel);
    writer_newline_exit(w);
    return 1;
}

int
fit_config_read(lexer_t *l, struct fit_config *config)
{
    if (lexer_check_ident(l, "fit-config")) goto config_exit;
    if (lexer_ident(l)) goto config_exit;
    if (strcmp(CSTR(l->store), "threshold") == 0) {
        config->threshold_given = 1;
        if (lexer_number(l, &config->chisq_threshold)) goto config_exit;
        if (lexer_ident(l)) goto config_exit;
    } else {
        config->threshold_given = 0;
    }
    if (strcmp(CSTR(l->store), "max-iterations")) goto config_exit;
    if (lexer_integer(l, &config->nb_max_iters)) goto config_exit;
    if (lexer_check_ident(l, "subsampling")) goto config_exit;
    if (lexer_integer(l, &config->subsampling)) goto config_exit;
    if (lexer_ident(l)) goto config_exit;
    if (strcmp(CSTR(l->store), "wavelength-range") == 0) {
        double lmin, lmax;
        if (lexer_number(l, &lmin)) goto config_exit;
        if (lexer_number(l, &lmax)) goto config_exit;
        config->spectr_range.active = 1;
        config->spectr_range.min = lmin;
        config->spectr_range.max = lmax;
        if (lexer_ident(l)) goto config_exit;
    } else {
        config->spectr_range.active = 0;
    }
    if (strcmp(CSTR(l->store), "epsilon")) goto config_exit;
    if (lexer_number(l, &config->epsabs)) goto config_exit;
    if (lexer_number(l, &config->epsrel)) goto config_exit;
    return 0;
config_exit:
    return 1;
}

void
fit_engine_get_cached_ns(struct fit_engine *fit, int j, cmpl ns[]) {
    const int nb_med = fit->stack->nb;
    const cmpl *ns_cached = fit->run->cache.ns_full_spectr + j * nb_med;
    for (int k = 0; k < nb_med; k++) {
        ns[k] = ns_cached[k];
    }
}
