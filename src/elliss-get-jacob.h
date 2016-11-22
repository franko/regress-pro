
/* HEADER file to be included directly in .c files */

extern void
get_parameter_jacobian(fit_param_t const *fp, stack_t const *stack,
                       struct deriv_info *ideriv, double lambda, double aoi_der[2],
                       gsl_vector *stack_jacob_th, cmpl_vector *stack_jacob_n,
                       struct elliss_ab *result);
