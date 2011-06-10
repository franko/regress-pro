/* -*- mode: C++; -*- */

#ifndef INTERATIVE_FIT_H
#define INTERATIVE_FIT_H

#include <fx.h>

#include <gsl/gsl_vector.h>

#include "agg_array.h"
#include "symtab.h"
#include "spectra.h"
#include "fit-params.h"
#include "fx_plot.h"

typedef agg::pod_auto_vector<plot *, 2> vector_2;

class InteractiveFit : public FXMainWindow {
  FXDECLARE(InteractiveFit)

  class plot_info : public vector_2 {
  public:
    ~plot_info() {
      unsigned n = size(), j;
      for (j = 0; j < n; j++)
	delete value_at(j);
    };

    void init(FXApp *app, unsigned n) {
      for (unsigned j = size(); j < n; j++)
	add(new plot(app));
    }
  };

  struct parameters_info {
    int number;
    struct fit_parameters *parameters;
    gsl_vector* values;
    agg::pod_array<bool> select;
    double* base_ptr;

    parameters_info() : parameters(0) {}

    ~parameters_info() {
      fit_parameters_free(parameters);
      gsl_vector_free(values); 
    }

    void init(struct fit_parameters *p) {
      number = p->number;
      parameters = p;
      values = gsl_vector_alloc(number);
      base_ptr = values->data;
      select.resize(number);
      for (int k = 0; k < number; k++)
	select[k] = false;
    }
  };

private:
  FXCanvas *canvas;
  agg::pod_array<FXTextField*> m_params_text_field;

  struct fit_engine *fit_engine;
  plot_info m_plots;

  parameters_info m_parameters;

  struct spectrum *spectrum;

  bool m_canvas_is_dirty;

protected:
  FXMenuBar         *menubar;
  FXStatusBar       *statusbar;
  FXMenuPane        *fitmenu;

  void updatePlot(bool freeze_limits = false);
  void drawPlot();

protected:
  InteractiveFit(){};
private:
  InteractiveFit(const InteractiveFit&);
  InteractiveFit &operator=(const InteractiveFit&);

public:
  InteractiveFit(FXApp *app, struct symtab *symtab, struct spectrum *sp);
  virtual ~InteractiveFit();

  long onCmdParamSelect(FXObject*, FXSelector,void*);
  long onCmdParamChange(FXObject*, FXSelector,void*);
  long onCmdRunFit(FXObject*, FXSelector,void*);
  long onCmdPaint(FXObject*, FXSelector,void*);
  long onUpdCanvas(FXObject*, FXSelector,void*);

  enum {
    ID_PARAM_SELECT = FXMainWindow::ID_LAST,
    ID_PARAM_VALUE,
    ID_RUN_FIT,
    ID_CANVAS,
    ID_LAST
  };
};

#endif
