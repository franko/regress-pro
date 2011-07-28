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

    m_fit_engine->config->subsampling = 0;
    fit_engine_prepare (m_fit_engine, m_ref_spectr, 0);

    m_fit_parameters = fit_parameters_new ();
    m_all_parameters = fit_engine_get_all_parameters (m_fit_engine);

    m_model_spectr = fit_engine_alloc_spectrum (m_fit_engine);
    fit_engine_generate_spectrum (m_fit_engine, m_model_spectr);
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
    fit_engine_generate_spectrum (m_fit_engine, m_model_spectr);
  }

  virtual double get_parameter_value(unsigned k)
  {
    fit_param_t* fp = &m_all_parameters->values[k];
    return fit_engine_get_parameter_value(m_fit_engine, fp);
  }

  virtual void get_sampling(double& s_start, double& s_end, double& s_step)
  {
    s_start = 240.0;
    s_end   = 780.0;
    s_step  = 2.0;
  }

  virtual bool set_sampling(double s_start, double s_end, double s_step)
  {
    return true;
  }

  virtual void run(struct fit_parameters* fps)
  {
    fit_engine_set_parameters (m_fit_engine, fps);

    gsl_vector* x = gsl_vector_alloc(fps->number);

    for (unsigned k = 0; k < fps->number; k++)
    {
      fit_param_t* fp = fps->values + k;
      double val = fit_engine_get_parameter_value(m_fit_engine, fp);
      gsl_vector_set (x, k, val);
    }

    double chisq;
    lmfit_simple (m_fit_engine, x, &chisq, 0, 0, 0, 0, 0);

    fit_engine_generate_spectrum (m_fit_engine, m_model_spectr);

    gsl_vector_free (x);
  }

  virtual void config_plot(plot_canvas* canvas)
  {
    spectra_plot (canvas, m_ref_spectr, m_model_spectr);
  }

  virtual ~interactive_fit() 
  {
    fit_engine_disable(m_fit_engine);
    fit_engine_free(m_fit_engine);
    fit_parameters_free(m_fit_parameters);
    fit_parameters_free(m_all_parameters);
    spectra_free(m_ref_spectr);
    spectra_free(m_model_spectr);
  }

private:
  struct fit_engine* m_fit_engine;
  struct spectrum* m_ref_spectr;
  struct spectrum* m_model_spectr;
  struct fit_parameters *m_fit_parameters;
  struct fit_parameters *m_all_parameters;
};

#endif
