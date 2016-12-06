
/* HEADER file to be included directly in .c files */

extern void
get_parameter_jacobian(fit_param_t const *fp, stack_t const *stack,
                       struct deriv_info *ideriv, double lambda,
                       gsl_vector *stack_jacob_th, cmpl_vector *stack_jacob_n,
                       double *jacob_acquisition, struct elliss_ab *result);
