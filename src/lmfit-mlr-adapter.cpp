#include "lmfit-mlr-adapter.h"
#include "elliss.h"
#include "refl-kernel.h"

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

void select_param_jacobian(const enum system_kind sys_kind, const int channels_number, const fit_param_t *fp, const stack_t *stack,
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
            result[q] = std::real(drdn) * dnr - std::imag(drdn) * dni;
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
        std::fill_n(result, channels_number, 0.0);
    }
}

void mult_layer_refl_sys(const enum system_kind sys_kind, int nb_med, const cmpl ns[],
                         const double ds[], double lambda,
                         const acquisition_parameters *acquisition, double result[],
                         double_array& jacob_th, cmpl_array& jacob_n, double_array& jacob_acq,
                         const unsigned jacob_flags)
{
    double_array *jacob_th_ptr  = (jacob_flags & REQUIRE_JACOB_T ? &jacob_th  : nullptr);
    cmpl_array   *jacob_n_ptr   = (jacob_flags & REQUIRE_JACOB_N ? &jacob_n   : nullptr);
    double_array *jacob_acq_ptr = (jacob_flags & REQUIRE_JACOB_A ? &jacob_acq : nullptr);

    switch (sys_kind) {
    case SYSTEM_SR:
    {
        mult_layer_refl_sr(nb_med, ns, ds, lambda, acquisition, result, jacob_th_ptr, jacob_n_ptr, jacob_acq_ptr);
        break;
    }
    case SYSTEM_SE_RPE:
    case SYSTEM_SE_RAE:
    case SYSTEM_SE:
    {
        const enum se_type se_type = SE_TYPE(sys_kind);
        ell_ab_t e;
        mult_layer_refl_se(se_type, nb_med, ns, ds, lambda, acquisition, e, jacob_th_ptr, jacob_n_ptr, jacob_acq_ptr);
        result[0] = e->alpha;
        result[1] = e->beta;
        break;
    }
    default:
        break;
    }
}
