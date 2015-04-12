#ifndef EMPTY_FIT_MANAGER_H
#define EMPTY_FIT_MANAGER_H

#include "fit_manager.h"

struct empty_fit_manager : fit_manager {
    virtual unsigned parameters_number() { return 0; }
    virtual double get_parameter(unsigned k, fit_param_t* fp) { return 0.0; }

    virtual void   set_parameter_value(unsigned k, double val) { }
    virtual double get_parameter_value(unsigned k) { return 0.0; }

    virtual void get_sampling(double& s_start, double& s_end, double& s_step) {
        s_start = 250.0;
        s_end = 750.0;
        s_step = 0.0;
    }

    virtual bool set_sampling(double s_start, double  s_end, double  s_step) { return true; }

    virtual void run(struct fit_parameters* fps) { }

    virtual void config_plot(plot_canvas* canvas) { }
};

#endif
