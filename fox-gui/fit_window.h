
/* fit_window.h
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

  long on_cmd_param_select(FXObject*, FXSelector,void*);
  long on_cmd_param_change(FXObject*, FXSelector,void*);
  long on_update_param(FXObject*, FXSelector,void*);
  long on_cmd_plot_autoscale(FXObject*, FXSelector,void*);
  long on_cmd_spectral_range(FXObject*, FXSelector,void*);
  long on_change_spectral_range(FXObject*, FXSelector,void*);
  long on_cmd_run_fit(FXObject*, FXSelector,void*);

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
