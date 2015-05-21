#ifndef FIT_MANAGER_H
#define FIT_MANAGER_H

#include "fit-params.h"
#include "plot_canvas.h"

struct fit_manager {
    virtual unsigned parameters_number() = 0;
    virtual double get_parameter(unsigned k, fit_param_t* fp) = 0;

    virtual void   set_parameter_value(unsigned k, double val) = 0;
    virtual double get_parameter_value(unsigned k) = 0;

    virtual void get_sampling(double& s_start, double& s_end, double& s_step) = 0;
    virtual bool set_sampling(double  s_start, double  s_end, double  s_step) = 0;

    virtual void run(struct fit_parameters* fps) = 0;

    virtual void config_plot(plot_canvas* canvas) = 0;

    virtual ~fit_manager() {}

    int lookup(const fit_param_t *fp) {
        for (unsigned k = 0; k < parameters_number(); k++) {
            fit_param_t fpk;
            get_parameter(k, &fpk);
            if (fit_param_compare(&fpk, fp) == 0) {
                return k;
            }
        }
        return -1;
    }
};

#endif
