extern double
get_parameter_jacob_r(fit_param_t const *fp,
                      stack_t const *stack,
                      struct deriv_info *ideriv, double lambda,
                      gsl_vector *stack_jacob_th,
                      gsl_vector *stack_jacob_n, double jacob_acq[1],
                      double rmult, double r_raw);
