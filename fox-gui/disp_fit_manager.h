
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

class disp_fit_manager : public fit_manager {
public:
  disp_fit_manager(struct disp_fit_engine* fit) :
    m_fit_engine(fit), m_sampling(240.0, 780.0, 271) 
  { 
    set_fit_param (&m_fp_template, 0);
  }

  virtual ~disp_fit_manager() {
    disp_fit_engine_free (m_fit_engine);
  }

  virtual unsigned parameters_number()
  { 
    return disp_get_number_of_params (m_fit_engine->model_disp); 
  }

  virtual double get_parameter(unsigned k, fit_param_t* fp)
  {
    disp_t* d = m_fit_engine->model_disp;
    assert(int(k) >= 0);
    set_fit_param (fp, int(k));
    return disp_get_param_value(d, fp);
  }

  virtual void set_parameter_value(unsigned k, double val)
  {
    disp_t* d = m_fit_engine->model_disp;
    m_fp_template.param_nb = int(k);
    dispers_apply_param (d, &m_fp_template, val);
  }

  virtual double get_parameter_value(unsigned k)
  {
    disp_t* d = m_fit_engine->model_disp;
    m_fp_template.param_nb = int(k);
    return disp_get_param_value(d, &m_fp_template);
  }

  virtual void get_sampling(double& s_start, double& s_end, double& s_step)
  {
    s_start = m_sampling.start();
    s_end   = m_sampling.end();
    s_step  = m_sampling.stride();
  }

  virtual bool set_sampling(double s_start, double s_end, double s_step)
  {
    if (s_step == 0.0) s_step = 1.0;
    unsigned npt = (s_end - s_start) / s_step;
    if (npt > 1 && npt <= 65536)
      {
	m_sampling.set(s_start, s_end, npt);
	return true;
      }
    return false;
  }

  virtual void run(struct fit_parameters* fps)
  {
    disp_t* d = m_fit_engine->model_disp;

    gsl_vector* x = gsl_vector_alloc(fps->number);

    for (unsigned k = 0; k < fps->number; k++)
    {
      double val = disp_get_param_value (d, &fps->values[k]);
      gsl_vector_set (x, k, val);
    }

    if (! m_fit_engine->wl)
      {
	m_fit_engine->wl = gsl_vector_alloc(m_sampling.size());
      }
    else
      {
	if (m_fit_engine->wl->size != m_sampling.size())
	  {
	    gsl_vector_free (m_fit_engine->wl);
	    m_fit_engine->wl = gsl_vector_alloc(m_sampling.size());
	  }
      }

    for (unsigned k = 0; k < m_sampling.size(); k++)
      gsl_vector_set (m_fit_engine->wl, k, m_sampling[k]);

    disp_fit_engine_set_parameters (m_fit_engine, fps);

    struct disp_fit_config cfg[1];
    disp_fit_config_init (cfg);

    double chisq;
    lmfit_disp (m_fit_engine, cfg, x, &chisq, 0, 0);

    gsl_vector_free(x);
  }

  virtual void config_plot(plot_canvas* canvas)
  {
    disp_t* model = m_fit_engine->model_disp;
    disp_t* ref   = m_fit_engine->ref_disp;

    disp_vs<sampling_unif>* ref_n = new disp_vs<sampling_unif>(ref,   cmpl::real_part, m_sampling);
    disp_vs<sampling_unif>* mod_n = new disp_vs<sampling_unif>(model, cmpl::real_part, m_sampling);
    add_new_plot(canvas, ref_n, mod_n, "refractive index");

    disp_vs<sampling_unif>* ref_k = new disp_vs<sampling_unif>(ref,   cmpl::imag_part, m_sampling);
    disp_vs<sampling_unif>* mod_k = new disp_vs<sampling_unif>(model, cmpl::imag_part, m_sampling);
    add_new_plot(canvas, ref_k, mod_k, "absorption coeff");
  }

private:
  void set_fit_param (fit_param_t* fp, int param_nb)
  {
    fp->id       = PID_LAYER_N;
    fp->layer_nb = 0;
    fp->model_id = m_fit_engine->model_disp->dclass->model_id;
    fp->param_nb = param_nb;
  }

  struct disp_fit_engine* m_fit_engine;
  sampling_unif m_sampling;
  fit_param_t m_fp_template;
};

#endif
