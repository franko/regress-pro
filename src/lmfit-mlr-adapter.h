#ifndef REGPRO_LMFIT_MR_ADAPTER_H
#define REGPRO_LMFIT_MR_ADAPTER_H

#include "acquisition.h"
#include "dispers.h"
#include "fit-params.h"
#include "stack.h"
#include "cmpl.h"

enum {
    REQUIRE_JACOB_NONE = 0,
    REQUIRE_JACOB_T = 1 << 0, // Require Jacobian for thickness.
    REQUIRE_JACOB_N = 1 << 1, // Require Jacobian for refractive index.
    REQUIRE_JACOB_A = 1 << 2, // Require Jacobian for acquisition parameters.
};

extern void select_param_jacobian(const enum system_kind sys_kind, const int channels_number, const fit_param_t *fp, const stack_t *stack,
                                  struct deriv_info *ideriv, double lambda,
                                  double result[], double jacob_th[], cmpl jacob_n[], double jacob_acq[]);

extern void mult_layer_refl_sys(const enum system_kind sys_kind, int nb_med, const cmpl ns[],
                                const double ds[], double lambda,
                                const acquisition_parameters *acquisition, double result[],
                                double_array *jacob_th, cmpl_array *jacob_n, double_array *jacob_acq,
                                const unsigned jacob_flags);

#endif
