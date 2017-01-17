
/* HEADER file to be included directly in .c files */

extern void
get_parameter_jacobian(fit_param_t const *fp, stack_t const *stack,
                       struct deriv_info *ideriv, double lambda,
                       double jacob_th[], cmpl jacob_n[],
                       double jacob_acq[], struct elliss_ab *result);
