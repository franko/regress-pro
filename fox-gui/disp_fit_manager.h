
/* disp_fit_manager.h
 *
 * Copyright (C) 2005-2011 Francesco Abbate
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DISP_FIT_MANAGER_H
#define DISP_FIT_MANAGER_H

#include <fx.h>

#include "fit_manager.h"
#include "sampling.h"
#include "disp-fit-engine.h"
#include "disp_vs.h"

/* It v == NULL this function will just count the number of points. */
static int interval_set_sampling(const disp_t *d, gsl_vector *v, double wavelength_start, double wavelength_end)
{
    const double wavelength_tol = 1.0E-10 * (wavelength_end - wavelength_start);
    const int ref_samples_number = disp_samples_number(d);
    int i = 0, k;
    for(k = 0; k < ref_samples_number; k++) {
        double wavelength = disp_sample_wavelength(d, k);
        if (wavelength > wavelength_start) break;
    }
    if (fabs(disp_sample_wavelength(d, k) - wavelength_start) > wavelength_tol) {
        if (v) {
            gsl_vector_set(v, i, wavelength_start);
        }
        i ++;
    }
    for(/* */; k < ref_samples_number; k++) {
        double wavelength = disp_sample_wavelength(d, k);
        if (wavelength > wavelength_end) break;
        if (v) {
            gsl_vector_set(v, i, wavelength);
        }
        i ++;
    }
    if (k > 0 && fabs(disp_sample_wavelength(d, k - 1) - wavelength_end) > wavelength_tol) {
        if (v) {
            gsl_vector_set(v, i, wavelength_end);
        }
        i ++;
    }
    return i;
}

static int interval_samples_number(const disp_t *d, double wavelength_start, double wavelength_end)
{
    if (disp_is_tabular(d)) {
        return interval_set_sampling(d, NULL, wavelength_start, wavelength_end);
    }
    return 0;
}

class disp_fit_manager : public fit_manager {
public:
    // Number of sampling points used for the fit.
    enum { SAMPLING_POINTS = 512 };

    disp_fit_manager(struct disp_fit_engine* fit) :
        m_fit_engine(fit) {
        set_default_sampling();
        set_fit_param(&m_fp_template, 0);
    }

    virtual ~disp_fit_manager() {
        disp_fit_engine_free(m_fit_engine);
    }

    virtual unsigned parameters_number() {
        return disp_get_number_of_params(m_fit_engine->model_disp);
    }

    virtual double get_parameter(unsigned k, fit_param_t* fp) {
        disp_t* d = m_fit_engine->model_disp;
        assert(int(k) >= 0);
        set_fit_param(fp, int(k));
        return disp_get_param_value(d, fp);
    }

    virtual void set_parameter_value(unsigned k, double val) {
        disp_t* d = m_fit_engine->model_disp;
        m_fp_template.param_nb = int(k);
        dispers_apply_param(d, &m_fp_template, val);
    }

    virtual double get_parameter_value(unsigned k) {
        disp_t* d = m_fit_engine->model_disp;
        m_fp_template.param_nb = int(k);
        return disp_get_param_value(d, &m_fp_template);
    }

    virtual void get_sampling(double& s_start, double& s_end) {
        s_start = m_sampling.start();
        s_end   = m_sampling.end();
    }

    virtual bool set_sampling(double s_start, double s_end) {
        if (s_end > s_start) {
            m_sampling.set(s_start, s_end, SAMPLING_POINTS);
            return true;
        }
        return false;
    }

    virtual lmfit_result run(fit_parameters* fps) {
        lmfit_result result;
        disp_t* d = m_fit_engine->model_disp;
        gsl_vector* x = gsl_vector_alloc(fps->number);

        for(int k = 0; k < fps->number; k++) {
            double val = disp_get_param_value(d, &fps->at(k));
            gsl_vector_set(x, k, val);
        }

        int ref_samples_number = interval_samples_number(m_fit_engine->ref_disp, m_sampling.start(), m_sampling.end());
        size_t wavelength_array_size = (ref_samples_number > 0 ? ref_samples_number : m_sampling.size());

        if(! m_fit_engine->wl) {
            m_fit_engine->wl = gsl_vector_alloc(wavelength_array_size);
        } else {
            if(m_fit_engine->wl->size != wavelength_array_size) {
                gsl_vector_free(m_fit_engine->wl);
                m_fit_engine->wl = gsl_vector_alloc(wavelength_array_size);
            }
        }

        if (ref_samples_number > 0) {
            interval_set_sampling(m_fit_engine->ref_disp, m_fit_engine->wl, m_sampling.start(), m_sampling.end());
        } else {
            for(unsigned k = 0; k < m_sampling.size(); k++) {
                gsl_vector_set(m_fit_engine->wl, k, m_sampling[k]);
            }
        }

        disp_fit_engine_set_parameters(m_fit_engine, fps);

        struct disp_fit_config cfg[1];
        disp_fit_config_init(cfg);

        lmfit_disp(m_fit_engine, cfg, x, &result, 0, 0);

        gsl_vector_free(x);
        return result;
    }

    virtual void config_plot(plot_canvas* canvas) {
        disp_t* model = m_fit_engine->model_disp;
        disp_t* ref   = m_fit_engine->ref_disp;

        disp_vs<sampling_unif>* ref_n = new disp_vs<sampling_unif>(ref,   REAL_PART, m_sampling);
        disp_vs<sampling_unif>* mod_n = new disp_vs<sampling_unif>(model, REAL_PART, m_sampling);
        add_new_plot(canvas, ref_n, mod_n, "refractive index");

        disp_vs<sampling_unif>* ref_k = new disp_vs<sampling_unif>(ref,   IMAGINARY_PART, m_sampling);
        disp_vs<sampling_unif>* mod_k = new disp_vs<sampling_unif>(model, IMAGINARY_PART, m_sampling);
        add_new_plot(canvas, ref_k, mod_k, "absorption coeff");
    }

    void set_default_sampling() {
        double wavelength_start, wavelength_end;
        int samples_number;
        disp_get_wavelength_range(m_fit_engine->ref_disp, &wavelength_start, &wavelength_end, &samples_number);
        m_sampling.set(wavelength_start, wavelength_end, samples_number);
    }

    void set_reference(disp_t *d) {
        disp_free(m_fit_engine->ref_disp);
        m_fit_engine->ref_disp = d;
        set_default_sampling();
    }

    void set_model(disp_t *d) {
        disp_free(m_fit_engine->model_disp);
        m_fit_engine->model_disp = d;
    }

    disp_t * model_ref() {
        return m_fit_engine->model_disp;
    }

    void set_model_ref(disp_t *d) {
        m_fit_engine->model_disp = d;
    }

private:
    void set_fit_param(fit_param_t* fp, int param_nb) {
        fp->id       = PID_LAYER_N;
        fp->layer_nb = 0;
        fp->model_id = m_fit_engine->model_disp->dclass->disp_class_id;
        fp->param_nb = param_nb;
    }

    struct disp_fit_engine* m_fit_engine;
    sampling_unif m_sampling;
    fit_param_t m_fp_template;
};

#endif
