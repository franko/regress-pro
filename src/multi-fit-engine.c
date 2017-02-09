#include <assert.h>
#include <string.h>

#include "defs.h"
#include "fit-engine.h"
#include "error-messages.h"
#include "multi-fit-engine.h"
#include "refl-kernel.h"
#include "elliss.h"

static int  mengine_apply_param_common(struct multi_fit_engine *fit,
                                       const fit_param_t *fp,
                                       double val);

static int  mengine_apply_param_priv(struct multi_fit_engine *fit,
                                     const fit_param_t *fp,
                                     double val, int sample);

static void build_multi_fit_engine_cache(struct multi_fit_engine *f);

static void dispose_multi_fit_engine_cache(struct multi_fit_engine *f);

void
build_multi_fit_engine_cache(struct multi_fit_engine *f)
{
    const int RI_IS_VARIABLE = 0;

    /* We have just one cache for the fit engine.
       A cache for each sample is not needed because we assume that
       the RI are not fixed and so we don't do presampling of n values */
    build_stack_cache(& f->cache, f->stack_list[0],
                      f->spectra_list[0], RI_IS_VARIABLE, 1);
}

static inline int se_acquisition_enum(int pid) {
    switch(pid) {
    case PID_AOI:       return SE_AOI;
    case PID_ANALYZER:  return SE_ANALYZER;
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
    if (sys_kind == SYSTEM_ELLISS_AB || sys_kind == SYSTEM_ELLISS_PSIDEL) {
        const int index = se_acquisition_enum(pid);
        return jacob_acq[SE_ACQ_INDEX(channel, index)];
    } else if (sys_kind == SYSTEM_REFLECTOMETER) {
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
        get_model_param_deriv(stack->disp[layer],
                              &ideriv[layer], fp, lambda,
                              &dnr, &dni);
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

static void
mult_layer_refl_sys(const enum system_kind sys_kind, size_t nb, const cmpl ns[],
                   const double ds[], double lambda,
                   const struct acquisition_parameters *acquisition, double result[],
                   double *jacob_th, cmpl *jacob_n, double *jacob_acq)
{
    switch (sys_kind) {
    case SYSTEM_REFLECTOMETER:
    {
        mult_layer_refl_sr(nb, ns, ds, lambda, acquisition, result, jacob_th, jacob_n, jacob_acq);
        break;
    }
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
    {
        const enum se_type se_type = GET_SE_TYPE(sys_kind);
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

static int
multifit_fdf(const gsl_vector *x, void *params, gsl_vector *f, gsl_matrix *jacob)
{
    struct multi_fit_engine *fit = params;
    const int samples_number = fit->samples_number;

    /* STEP 1 : We apply the actual values of the fit parameters
                to the stack. */

    multi_fit_engine_commit_parameters(fit, x);

    int sample_offset = 0;
    for(int sample = 0; sample < samples_number; sample++) {
        struct spectrum *spectrum = fit->spectra_list[sample];
        struct stack *stack_sample = fit->stack_list[sample];
        const int nb_med = stack_sample->nb;
        const int npt = spectra_points(spectrum);
        const enum system_kind sys_kind = spectrum->acquisition->type;
        const int channels_number = SYSTEM_CHANNELS_NUMBER(sys_kind);
        double jacob_th_data[channels_number * (nb_med - 2)];
        cmpl jacob_n_data[channels_number * nb_med];
        double jacob_acq_data[channels_number * SYSTEM_ACQUISITION_PARAMS_NUMBER(sys_kind)];

        /* STEP 2 : From the stack we retrive the thicknesses and RIs
        informations. */

        const double *ths = stack_get_ths_list(stack_sample);

        /* TODO: take into account the case of fixed RIs and when acquisition
           parameters are not needed. */
        double *jacob_th  = (jacob ? jacob_th_data  : NULL);
        cmpl *  jacob_n   = (jacob ? jacob_n_data   : NULL);
        double *jacob_acq = (jacob ? jacob_acq_data : NULL);

        for(int j = 0, jpt = sample_offset; j < npt; j++, jpt += channels_number) {
            float const * spectr_data = spectra_get_values(spectrum, j);
            const double lambda = spectr_data[0];
            double result[channels_number];

            cmpl ns[nb_med];
            stack_get_ns_list(stack_sample, ns, lambda);

            /* STEP 3 : We call the ellipsometer kernel function */
            mult_layer_refl_sys(sys_kind, nb_med, ns, ths, lambda, &fit->acquisitions[sample], result, jacob_th, jacob_n, jacob_acq);

            if(f != NULL) {
                for (int q = 0; q < channels_number; q++) {
                    gsl_vector_set(f, jpt + q, result[q] - spectr_data[q + 1]);
                }
                if (lambda < 194) {
                    if (sample == 0) {
                        printf("wavelength: %g\n", lambda);
                    }
                    printf("sample: %d", sample);
                    for (int q = 0; q < channels_number; q++) {
                        printf(" f[%d(%d)]= (%12.6g : %12.6g)", j, q, result[q], spectr_data[q + 1]);
                    }
                    printf("\n");
                }
            }

            if(jacob) {
                struct deriv_info * ideriv = fit->cache.deriv_info;
                const int nb_comm_params = fit->common_parameters->number;
                const int nb_priv_params = fit->private_parameters->number;
                const int nb_params = nb_comm_params + nb_priv_params * samples_number;

                for(int ic = 0; ic < nb_med; ic++) {
                    ideriv[ic].is_valid = 0;
                }

                if (lambda < 194) {
                    printf("JACOB (common) sample: %d\n", sample);
                }

                int kp;
                for(kp = 0; kp < nb_comm_params; kp++) {
                    const fit_param_t *fp = fit->common_parameters->values + kp;
                    double fp_jacob[channels_number];

                    select_param_jacobian(sys_kind, channels_number, fp, stack_sample, ideriv, lambda,
                                          fp_jacob, jacob_th, jacob_n, jacob_acq);

                    for (int q = 0; q < channels_number; q++) {
                        gsl_matrix_set(jacob, jpt + q, kp, fp_jacob[q]);
                    }

                    if (lambda < 194) {
                        for (int q = 0; q < channels_number; q++) {
                            printf(" JACOB[%d(%d), %d] = %12.6g", j, q, kp, fp_jacob[q]);
                        }
                        printf("\n");
                    }
                }

                for(int ikp = 0; ikp < nb_priv_params * sample; ikp++, kp++) {
                    for (int q = 0; q < channels_number; q++) {
                        gsl_matrix_set(jacob, jpt + q, kp, 0.0);
                    }
                }

                if (lambda < 194) {
                    printf("JACOB (priv) sample: %d\n", sample);
                }

                for(int ikp = 0; ikp < nb_priv_params; kp++, ikp++) {
                    fit_param_t *fp = fit->private_parameters->values + ikp;
                    double fp_jacob[channels_number];

                    select_param_jacobian(sys_kind, channels_number, fp, stack_sample, ideriv, lambda,
                                          fp_jacob, jacob_th, jacob_n, jacob_acq);

                    for (int q = 0; q < channels_number; q++) {
                        gsl_matrix_set(jacob, jpt + q, kp, fp_jacob[q]);
                    }

                    if (lambda < 194) {
                        for (int q = 0; q < channels_number; q++) {
                            printf(" JACOB[%d(%d), %d] = %12.6g", j, q, kp, fp_jacob[q]);
                        }
                        printf("\n");
                    }
                }

                for(/* */; kp < nb_params; kp++) {
                    for (int q = 0; q < channels_number; q++) {
                        gsl_matrix_set(jacob, jpt + q, kp, 0.0);
                    }
                }
            }
        }

        sample_offset += channels_number * npt;
    }

    return GSL_SUCCESS;
}

static int
multifit_f(const gsl_vector *x, void *params, gsl_vector *f)
{
    return multifit_fdf(x, params, f, NULL);
}

static int
multifit_df(const gsl_vector *x, void *params, gsl_matrix *jacob)
{
    return multifit_fdf(x, params, NULL, jacob);
}

int
multi_fit_engine_prepare(struct multi_fit_engine *fit)
{
    struct fit_parameters const * common = fit->common_parameters;
    struct fit_parameters const * priv   = fit->private_parameters;
    struct fit_config *cfg = & fit->config;
    size_t nb_total_params;

    assert(fit->spectra_list != NULL);

    /* Copy all the spectra acquisitions info into the fit structure. */
    for (int j = 0; j < fit->samples_number; j++) {
        fit->acquisitions[j] = fit->spectra_list[j]->acquisition[0];
    }

    nb_total_params = common->number + fit->samples_number * priv->number;


    if(fit->config.spectr_range.active) {
        int k;
        for(k = 0; k < fit->samples_number; k++)
            spectr_cut_range(fit->spectra_list[k],
                             fit->config.spectr_range.min,
                             fit->config.spectr_range.max);
    }

    build_multi_fit_engine_cache(fit);

    cfg->chisq_threshold = 0.0;
    int npt_sum = 0;
    for(int k = 0; k < fit->samples_number; k++) {
        const struct spectrum *spectrum = fit->spectra_list[k];
        const enum system_kind sys_kind = spectrum->acquisition->type;
        npt_sum += SYSTEM_CHANNELS_NUMBER(sys_kind) * spectra_points(spectrum);
        cfg->chisq_threshold = (sys_kind == SYSTEM_REFLECTOMETER ? 5.0E3 : 2.0E5);
    }

    fit->mffun.f      = &multifit_f;
    fit->mffun.df     = &multifit_df;
    fit->mffun.fdf    = &multifit_fdf;
    fit->mffun.n      = npt_sum;
    fit->mffun.p      = nb_total_params;
    fit->mffun.params = fit;

    fit->results = gsl_vector_alloc(nb_total_params);
    fit->chisq   = gsl_vector_alloc(fit->samples_number);

    fit->initialized = 1;

    return 0;
}

void
dispose_multi_fit_engine_cache(struct multi_fit_engine *f)
{
    dispose_stack_cache(& f->cache);
}

void
multi_fit_engine_disable(struct multi_fit_engine *fit)
{
    dispose_multi_fit_engine_cache(fit);

    gsl_vector_free(fit->results);
    fit->results = NULL;

    gsl_vector_free(fit->chisq);
    fit->chisq = NULL;

    fit->initialized = 0;
}

int
mengine_apply_param_common(struct multi_fit_engine *fit,
                           const fit_param_t *fp,
                           double val)
{
    int res = 0;
    for(int j = 0; j < fit->samples_number; j++) {
        if (fp->id >= PID_ACQUISITION_PARAMETER) {
            res = acquisition_apply_param(&fit->acquisitions[j], fp->id, val);
        } else {
            res = stack_apply_param(fit->stack_list[j], fp, val);
        }
        if(res) {
            break;
        }
    }
    return res;
}

int
mengine_apply_param_priv(struct multi_fit_engine *fit,
                         const fit_param_t *fp,
                         double val, int sample)
{
    int res = 0;

    switch(fp->id) {
    case PID_FIRSTMUL:
        res = 1;
        break;
    default:
        res = stack_apply_param(fit->stack_list[sample], fp, val);
    }

    return res;

}

int
multi_fit_engine_commit_parameters(struct multi_fit_engine *fit,
                                   const gsl_vector *x)
{
    struct fit_parameters const * common = fit->common_parameters;
    struct fit_parameters const * priv   = fit->private_parameters;
    size_t nb_priv_params = priv->number;
    size_t j, joffs = 0;
    int sample;

    for(j = 0; j < common->number; j++) {
        const fit_param_t *fp = common->values + j;
        double param_value = gsl_vector_get(x, joffs + j);
        int status;

        status = mengine_apply_param_common(fit, fp, param_value);

        /* No error should never occurs here because the fit parameters
        are checked in advance. */
        assert(status == 0);
    }

    joffs += common->number;

    for(j = 0; j < nb_priv_params; j++) {
        const fit_param_t *fp = priv->values + j;

        for(sample = 0; sample < fit->samples_number; sample++) {
            double param_value =
                gsl_vector_get(x, joffs + nb_priv_params * sample + j);
            int status;

            status = mengine_apply_param_priv(fit, fp, param_value, sample);

            assert(status == 0);
        }
    }

    return 0;
}

struct multi_fit_engine *
multi_fit_engine_new(struct fit_config const *cfg, int samples_number) {
    struct multi_fit_engine *f;

    f = emalloc(sizeof(struct multi_fit_engine));

    f->samples_number = samples_number;
    f->stack_list = emalloc(samples_number * sizeof(void *));
    f->spectra_list = emalloc(samples_number * sizeof(void *));
    f->acquisitions  = emalloc(samples_number * sizeof(struct acquisition_parameters));

    memcpy(&f->config, cfg, sizeof(struct fit_config));

    /* fit parameters are provided later */
    f->common_parameters = NULL;
    f->private_parameters = NULL;

    f->results = NULL;

    f->initialized = 0;

    return f;
}

void
multi_fit_engine_free(struct multi_fit_engine *f)
{
    int k;

    if(f->stack_list) {
        for(k = 0; k < f->samples_number; k++) {
            if(f->stack_list[k] != NULL) {
                stack_free(f->stack_list[k]);
            }
        }
        free(f->stack_list);
    }


    /* we don't free each spectrum because we are not the owner */
    free(f->spectra_list);

    /* we don't free f->common_parameters and f->private_parameters
       because multi_fit_engine does not own these data */

    assert(f->initialized == 0);

    free(f);
}

void
multi_fit_engine_bind(struct multi_fit_engine *fit, const stack_t *stack, const struct fit_parameters *cparameters, const struct fit_parameters *pparameters)
{
    int k;
    /* fit is not the owner of the "parameters", we just keep a reference */
    fit->common_parameters = cparameters;

    for(k = 0; k < fit->samples_number; k++) {
        fit->stack_list[k] = stack_copy(stack);
    }

    /* fit is not the owner of the "parameters", we just keep a reference */
    fit->private_parameters = pparameters;
}

void
multi_fit_engine_apply_parameters(struct multi_fit_engine *fit, int sample_nb, const struct fit_parameters *fps, const double value[])
{
    int k;
    for(k = 0; k < fps->number; k++) {
        stack_apply_param(fit->stack_list[sample_nb], &fps->values[k], value[k]);
    }
}

void
multi_fit_engine_print_fit_results(struct multi_fit_engine *fit,
                                   str_t text)
{
    str_t pname;
    size_t k, j, kp;

    str_set_null(text);

    str_init(pname, 15);

    for(j = 0, kp = 0; j < fit->common_parameters->number; j++, kp++) {
        fit_param_t *fp = fit->common_parameters->values + j;
        get_param_name(fp, pname);
        str_printf_add(text, "COMMON    / %9s : %.6g\n", CSTR(pname),
                       gsl_vector_get(fit->results, j));
    }


    for(k = 0; k < fit->samples_number; k++) {
        for(j = 0; j < fit->private_parameters->number; j++, kp++) {
            fit_param_t *fp = fit->private_parameters->values + j;
            get_param_name(fp, pname);
            str_printf_add(text, "SAMPLE(%02i)/ %9s : %.6g\n", k, CSTR(pname),
                           gsl_vector_get(fit->results, kp));
        }
    }

    str_append_c(text, "Residual Chi Square by sample:\n", '\n');
    for(k = 0; k < fit->samples_number; k++) {
        double chisq = gsl_vector_get(fit->chisq, k);
        str_printf_add(text, "ChiSq(%02i): %g\n", k, chisq);
    }

    str_free(pname);
}

double
multi_fit_engine_get_parameter_value(const struct multi_fit_engine *fit, const fit_param_t *fp)
{
    if(fp->id >= PID_ACQUISITION_PARAMETER) {
        return acquisition_get_parameter(&fit->acquisitions[0], fp->id);
    } else {
        if (fit->stack_list[0]) {
            return stack_get_parameter_value(fit->stack_list[0], fp);
        }
    }
    return 0.0;
}

double
multi_fit_engine_get_seed_value(const struct multi_fit_engine *fit, const fit_param_t *fp, const seed_t *s)
{
    if (s->type == SEED_UNDEF) {
        return multi_fit_engine_get_parameter_value(fit, fp);
    }
    return s->seed;
}
