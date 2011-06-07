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

private:
    struct fit_parameters *params;
    gsl_vector *param_values;
  struct fit_engine *fit_engine;
  FXCanvas *canvas;

  plot_info m_plots;

  struct spectrum *spectrum;

protected:
  FXMenuBar         *menubar;
  FXStatusBar       *statusbar;
  FXMenuPane        *fitmenu;

  void updatePlot();
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
  long onCmdPaint(FXObject*, FXSelector,void*);

  enum {
    ID_PARAM_SELECT = FXMainWindow::ID_LAST,
    ID_PARAM_VALUE,
    ID_RUN_FIT,
    ID_CANVAS,
    ID_LAST
  };
};

#endif
