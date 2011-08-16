
/* interactive_fit.h
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

#ifndef INTERACTIVE_FIT_H
#define INTERACTIVE_FIT_H

#include <fx.h>

#include "fit_manager.h"
#include "fit-engine.h"
#include "lmfit-simple.h"
#include "spectrum_plot.h"

class interactive_fit : public fit_manager {
public:
  interactive_fit(struct fit_engine *fit, struct spectrum *user_spectr) :
    m_fit_engine(fit)
  { 
    m_ref_spectr = spectra_copy (user_spectr);

    m_all_parameters = fit_engine_get_all_parameters (m_fit_engine);

    m_fit_engine->config->spectr_range.active = 0;
    m_fit_engine->config->subsampling = 0;
    
    m_model_spectr = spectra_alloc (m_ref_spectr);
    fit_engine_generate_spectrum (m_fit_engine, m_ref_spectr, m_model_spectr);
  }

  virtual unsigned parameters_number()
  { 
    return m_all_parameters->number;
  }

  virtual double get_parameter(unsigned k, fit_param_t* fp)
  {
    *fp = m_all_parameters->values[k];
    return fit_engine_get_parameter_value(m_fit_engine, fp);
  }

  virtual void set_parameter_value(unsigned k, double val)
  {
    fit_param_t* fp = &m_all_parameters->values[k];
    fit_engine_apply_param (m_fit_engine, fp, val);
    fit_engine_generate_spectrum (m_fit_engine, m_ref_spectr, m_model_spectr);
  }

  virtual double get_parameter_value(unsigned k)
  {
    fit_param_t* fp = &m_all_parameters->values[k];
    return fit_engine_get_parameter_value(m_fit_engine, fp);
  }

  virtual void get_sampling(double& s_start, double& s_end, double& s_step)
  {
    int n = spectra_points(m_ref_spectr);
    s_start = data_view_get(m_ref_spectr->table, 0,   0);
    s_end   = data_view_get(m_ref_spectr->table, n-1, 0);
    s_step  = 0.0;
  }

  virtual bool set_sampling(double s_start, double s_end, double s_step)
  {
    spectr_cut_range(m_ref_spectr, s_start, s_end);

    spectra_free (m_model_spectr);
    m_model_spectr = spectra_alloc (m_ref_spectr);
    fit_engine_generate_spectrum (m_fit_engine, m_ref_spectr, m_model_spectr);

    return true;
  }

  virtual void run(struct fit_parameters* fps)
  {
    m_fit_engine->parameters = fps;

    fit_engine_prepare(m_fit_engine, m_ref_spectr);

    gsl_vector* x = gsl_vector_alloc(fps->number);

    for (unsigned k = 0; k < fps->number; k++)
    {
      fit_param_t* fp = fps->values + k;
      double val = fit_engine_get_parameter_value(m_fit_engine, fp);
      gsl_vector_set (x, k, val);
    }

    double chisq;
    lmfit_simple (m_fit_engine, x, &chisq, 0, 0, 0, 0, 0);

    fit_engine_generate_spectrum (m_fit_engine, m_ref_spectr, m_model_spectr);

    gsl_vector_free (x);

    fit_engine_disable (m_fit_engine);
  }

  virtual void config_plot(plot_canvas* canvas)
  {
    spectra_plot (canvas, m_ref_spectr, m_model_spectr);
  }

  virtual ~interactive_fit() 
  {
    fit_engine_free(m_fit_engine);
    fit_parameters_free(m_all_parameters);
    spectra_free(m_ref_spectr);
    spectra_free(m_model_spectr);
  }

private:
  struct fit_engine* m_fit_engine;
  struct spectrum* m_ref_spectr;
  struct spectrum* m_model_spectr;
  struct fit_parameters *m_all_parameters;
};

#endif
