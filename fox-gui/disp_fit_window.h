/* -*- mode: C++; -*- */

#ifndef DISP_FIT_WINDOW_H
#define DISP_FIT_WINDOW_H

#include <fx.h>

#include <gsl/gsl_vector.h>

#include "agg2/agg_array.h"

#include "EllissApp.h"
#include "disp-fit-engine.h"
#include "fit-params.h"
#include "fx_plot.h"

typedef agg::pod_auto_vector<plot *, 2> vector_2;

class disp_fit_window : public FXMainWindow {
  FXDECLARE(disp_fit_window)

  class plot_array : public vector_2 {
  public:
    ~plot_array() {
      unsigned n = size(), j;
      for (j = 0; j < n; j++)
	delete value_at(j);
    };

    void init(FXApp *app, unsigned n) {
      for (unsigned j = size(); j < n; j++)
	add(new plot(app));
    }
  };

  struct param_info {
    FXTextField *text_field;
    fit_param_t fp;
    double value;
    bool selected;
  };

private:
  agg::pod_array<param_info> m_parameters;

  struct disp_fit_engine *m_fit_engine;
  plot_array m_plots;

  struct fit_parameters *m_fit_parameters;

  bool m_canvas_is_dirty;

protected:
  FXMenuBar         *menubar;
  FXStatusBar       *statusbar;
  FXMenuPane        *fitmenu;

  FXCanvas *canvas;

  void updatePlot(bool freeze_limits = false);
  void drawPlot();

protected:
  disp_fit_window(){};
private:
  disp_fit_window(const disp_fit_window&);
  disp_fit_window &operator=(const disp_fit_window&);

public:
  disp_fit_window(EllissApp *app, struct disp_fit_engine *fit);
  virtual ~disp_fit_window();

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
