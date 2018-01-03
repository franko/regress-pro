extern double
get_parameter_jacob_r(fit_param_t const *fp,
                      stack_t const *stack,
                      struct deriv_info *ideriv, double lambda,
                      double_array jacob_th, cmpl_array jacob_n, double_array jacob_acq);
