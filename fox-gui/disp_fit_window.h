/* -*- mode: C++; -*- */

#ifndef DISP_FIT_WINDOW_H
#define DISP_FIT_WINDOW_H

#include <fx.h>

#include <gsl/gsl_vector.h>

#include "agg2/agg_array.h"

#include "elliss_app.h"
#include "disp-fit-engine.h"
#include "fit-params.h"
#include "sampling.h"
#include "agg_plot_array.h"
#include "plot_canvas.h"

class disp_fit_window : public FXMainWindow {
  FXDECLARE(disp_fit_window)

  struct param_info {
    FXTextField *text_field;
    fit_param_t fp;
    double value;
    bool selected;
  };

private:
  agg::pod_array<param_info> m_parameters;
  struct disp_fit_engine *m_fit_engine;
  struct fit_parameters *m_fit_parameters;
  sampling_unif m_wl_sampling;

protected:
  FXMenuBar         *menubar;
  FXStatusBar       *statusbar;
  FXMenuPane        *fitmenu;
  FXMenuPane        *plotmenu;

  FXTextField* m_wl_entry;

  plot_canvas* m_canvas;

  void init_engine();
  void config_plot();
  void update_disp();
  void draw_plot(FXEvent* event = 0);

  bool verify_spectral_range (const char *txt, double ps[]);
  bool update_spectral_range (const char *txt);
  void update_fit_engine_sampling ();

protected:
  disp_fit_window(){};
private:
  disp_fit_window(const disp_fit_window&);
  disp_fit_window &operator=(const disp_fit_window&);

public:
  disp_fit_window(elliss_app *app, struct disp_fit_engine *fit);
  virtual ~disp_fit_window();

  long onCmdParamSelect(FXObject*, FXSelector,void*);
  long onCmdParamChange(FXObject*, FXSelector,void*);
  long onCmdRunFit(FXObject*, FXSelector,void*);
  long onCmdPlotAutoScale(FXObject*, FXSelector,void*);
  long onCmdSpectralRange(FXObject*, FXSelector,void*);
  long onChangeSpectralRange(FXObject*, FXSelector,void*);

  enum {
    ID_PARAM_SELECT = FXMainWindow::ID_LAST,
    ID_PARAM_VALUE,
    ID_SPECTR_RANGE,
    ID_RUN_FIT,
    ID_PLOT_SCALE,
    ID_LAST
  };
};

#endif
