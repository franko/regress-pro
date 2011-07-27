
#include "fit_window.h"

#include "elliss_app.h"
#include "fx_numeric_field.h"
#include "Strcpp.h"

// Map
FXDEFMAP(fit_window) fit_window_map[]={
  FXMAPFUNC(SEL_COMMAND, fit_window::ID_PARAM_SELECT, fit_window::onCmdParamSelect),
  FXMAPFUNC(SEL_COMMAND, fit_window::ID_PARAM_VALUE,  fit_window::onCmdParamChange),
  FXMAPFUNC(SEL_CHANGED, fit_window::ID_PARAM_VALUE,  fit_window::onCmdParamChange),
  FXMAPFUNC(SEL_UPDATE,  fit_window::ID_PARAM_VALUE,  fit_window::onUpdParam),
  FXMAPFUNC(SEL_COMMAND, fit_window::ID_RUN_FIT,      fit_window::onCmdRunFit),
  FXMAPFUNC(SEL_COMMAND, fit_window::ID_PLOT_SCALE,   fit_window::onCmdPlotAutoScale),
  FXMAPFUNC(SEL_COMMAND, fit_window::ID_SPECTR_RANGE, fit_window::onCmdSpectralRange),
  FXMAPFUNC(SEL_CHANGED, fit_window::ID_SPECTR_RANGE, fit_window::onChangeSpectralRange),
};

// Object implementation
FXIMPLEMENT(fit_window,FXMainWindow,fit_window_map,ARRAYNUMBER(fit_window_map));

fit_window::fit_window(fit_manager* fit, FXApp* a,const FXString& name,FXIcon *ic,FXIcon *mi,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs)
  : FXMainWindow(a, "Dispersion Fit", ic, mi, opts, x, y, w, h, pl, pr, pt, pb, hs, vs),
    m_canvas(0), m_fit(fit)
{
  // Menubar
  menubar = new FXMenuBar(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  statusbar = new FXStatusBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|FRAME_RAISED|STATUSBAR_WITH_DRAGCORNER);

  // fit menu
  fitmenu = new FXMenuPane(this);
  new FXMenuCommand(fitmenu, "&Run", NULL, this, ID_RUN_FIT);
  new FXMenuTitle(menubar, "&Fit", NULL, fitmenu);

  // plot menu
  plotmenu = new FXMenuPane(this);
  new FXMenuCommand(plotmenu, "&Auto Scale", NULL, this, ID_PLOT_SCALE);
  new FXMenuTitle(menubar, "&Plot", NULL, plotmenu);

  FXHorizontalFrame *mf = new FXHorizontalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXScrollWindow *iw = new FXScrollWindow(mf, VSCROLLER_ALWAYS | HSCROLLING_OFF | LAYOUT_FILL_Y);

  FXMatrix *matrix = new FXMatrix(iw, 2, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_Y|MATRIX_BY_COLUMNS, 0, 0, 0, 0, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, 1, 1);

  new FXLabel(matrix, "Range");
  m_wl_entry = new FXTextField(matrix, 10, this, ID_SPECTR_RANGE, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_ROW);

  {
    double wls, wle, wld;
    m_fit->get_sampling(wls, wle, wld);
    m_wl_entry->setText(FXStringFormat("%g-%g,%g", wls, wle, wld));
  }

  m_parameters.resize(m_fit->parameters_number());

  Str pname;
  for (unsigned k = 0; k < m_parameters.size(); k++)
    {
      param_info* p = this->get_parameter_pointer(k);
      m_fit->get_parameter(k, &p->fp);

      get_param_name(&p->fp, pname.str());
      FXString fxpname((const FXchar *) pname.cstr());
      FXCheckButton* bt = new FXCheckButton(matrix, fxpname, this, ID_PARAM_SELECT);
      FXTextField* tf = new fx_numeric_field(matrix, 10, this, ID_PARAM_VALUE, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);

      tf->setUserData(p);
      bt->setUserData(p);

      p->text_field = tf;
      p->selected = false;
      p->is_dirty = true;
    }

  m_canvas = new plot_canvas(mf, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);

  m_fit->config_plot(m_canvas);
}

fit_window::~fit_window() 
{
  delete m_fit;
  delete fitmenu;
  delete plotmenu;
}

long
fit_window::onCmdParamSelect(FXObject* _cb, FXSelector, void*)
{
  FXCheckButton *cb = (FXCheckButton *) _cb;
  param_info* p_inf = (param_info*) cb->getUserData();
  p_inf->selected = cb->getCheck();
  return 1;
}

long
fit_window::onUpdParam(FXObject *_txt, FXSelector, void*)
{
  FXTextField *txt = (FXTextField *) _txt;
  param_info* p = (param_info*) txt->getUserData();
  if (p->is_dirty)
    {
      unsigned k = this->get_parameter_index(p);
      FXString s = FXStringFormat("%g", m_fit->get_parameter_value(k));
      txt->setText(s, false);
      p->is_dirty = false;
      return 1;
    }
  return 0;
}

long
fit_window::onCmdParamChange(FXObject *_txt, FXSelector, void*)
{
  FXTextField *txt = (FXTextField *) _txt;
  param_info* p = (param_info*) txt->getUserData();
  FXString vstr = txt->getText();
  double new_val = strtod (vstr.text(), NULL);
  unsigned k = this->get_parameter_index(p);
  m_fit->set_parameter_value(k, new_val);
  if (m_canvas) m_canvas->set_dirty(true);
  return 1;
}

long
fit_window::onCmdPlotAutoScale(FXObject*, FXSelector, void*)
{
  m_canvas->update_limits();
  return 1;
}

bool
fit_window::verify_spectral_range (const char *txt, double ps[])
{
  int nass = sscanf(txt, "%lf-%lf,%lf", ps, ps+1, ps+2);

  if (nass < 3) 
    return false;

  if (ps[2] < 0.1 || ps[0] < 50.0 || ps[1] > 5000.0 || ps[1] < ps[0])
    return false;

  if (int((ps[1]-ps[0])/ps[2]) < 2)
    return false;

  return true;
}

bool
fit_window::update_spectral_range (const char *txt)
{
  double ps[3];

  if (verify_spectral_range (txt, ps))
    {
      return m_fit->set_sampling(ps[0], ps[1], ps[2]);
    }

  return false;
}

long
fit_window::onChangeSpectralRange(FXObject *, FXSelector, void*_txt)
{
  const char * txt = (const char *) _txt;

  if (update_spectral_range (txt))
    {
      m_wl_entry->setTextColor(FXRGB(0,0,0));
      if (m_canvas) m_canvas->update_limits();
    }
  else
    {
      m_wl_entry->setTextColor(FXRGB(180,0,0));
    }

  return 1;
}

long
fit_window::onCmdSpectralRange(FXObject *, FXSelector, void*)
{
  FXString s = m_wl_entry->getText();
  if (update_spectral_range (s.text()))
    {
      if (m_canvas) m_canvas->update_limits();
      return 1;
    }
  return 0;
}

long
fit_window::onCmdRunFit(FXObject*, FXSelector, void* ptr)
{
  reg_check_point(this);

  struct fit_parameters* fps = fit_parameters_new();

  int fit_params_nb = 0;
  for (unsigned j = 0; j < m_parameters.size(); j++)
    {
      if (m_parameters[j].selected)
	{
	  fit_parameters_add (fps, &m_parameters[j].fp);
	  fit_params_nb++;
	}
    }

  m_fit->run(fps);

  for (unsigned k = 0; k < m_parameters.size(); k++)
    {
      param_info& p = m_parameters[k];
      FXString s = FXStringFormat("%g", m_fit->get_parameter_value(k));
      p.text_field->setText(s);
      p.is_dirty = true;
    }

  fit_parameters_free(fps);

  m_canvas->update_limits();
  return 1;
}
