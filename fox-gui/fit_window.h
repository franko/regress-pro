#ifndef FIT_WINDOW_H
#define FIT_WINDOW_H

#include <fx.h>
#include <agg2/agg_array.h>

#include "fit_manager.h"
#include "plot_canvas.h"

class fit_window : public FXMainWindow {
  FXDECLARE(fit_window)

  struct param_info {
    FXTextField *text_field;
    fit_param_t fp;
    bool selected;
    bool is_dirty;
  };

  agg::pod_array<param_info> m_parameters;

  FXMenuBar         *menubar;
  FXStatusBar       *statusbar;
  FXMenuPane        *fitmenu;
  FXMenuPane        *plotmenu;

  FXTextField* m_wl_entry;
  plot_canvas* m_canvas;

  fit_manager* m_fit;

  bool verify_spectral_range (const char *txt, double ps[]);
  bool update_spectral_range (const char *txt);

  param_info* get_parameter_pointer(unsigned k)  { return m_parameters.data() + k; }
  unsigned    get_parameter_index(param_info* p) { return p - m_parameters.data(); }

protected:
  fit_window(){};
private:
  fit_window(const fit_window&);
  fit_window &operator=(const fit_window&);

public:
  fit_window(fit_manager* fit,FXApp* a,const FXString& name,FXIcon *ic=NULL,FXIcon *mi=NULL,FXuint opts=DECOR_ALL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,FXint hs=0,FXint vs=0);
  virtual ~fit_window();

  long onCmdParamSelect(FXObject*, FXSelector,void*);
  long onCmdParamChange(FXObject*, FXSelector,void*);
  long onUpdParam(FXObject*, FXSelector,void*);
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
