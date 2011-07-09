
#include "interactive_fit.h"
#include "Strcpp.h"
#include "fit-engine.h"
#include "lmfit-simple.h"
#include "spectra-path.h"

// Map
FXDEFMAP(interactive_fit) interactive_fitMap[]={
  FXMAPFUNC(SEL_COMMAND, interactive_fit::ID_PARAM_SELECT, interactive_fit::onCmdParamSelect),
  FXMAPFUNC(SEL_COMMAND, interactive_fit::ID_PARAM_VALUE,  interactive_fit::onCmdParamChange),
  FXMAPFUNC(SEL_CHANGED, interactive_fit::ID_PARAM_VALUE,  interactive_fit::onCmdParamChange),
  FXMAPFUNC(SEL_PAINT,   interactive_fit::ID_CANVAS,       interactive_fit::onCmdPaint),
  FXMAPFUNC(SEL_UPDATE,  interactive_fit::ID_CANVAS,       interactive_fit::onUpdCanvas),
  FXMAPFUNC(SEL_COMMAND, interactive_fit::ID_RUN_FIT,      interactive_fit::onCmdRunFit),
};

// Object implementation
FXIMPLEMENT(interactive_fit,FXMainWindow,interactive_fitMap,ARRAYNUMBER(interactive_fitMap));

interactive_fit::interactive_fit(elliss_app *app, struct fit_engine *_fit, struct spectrum *user_spectr)
  : FXMainWindow(app, "Interactive Fit", NULL, &app->appicon, DECOR_ALL, 0, 0, 640, 480),
    fit_engine(_fit), spectrum(user_spectr), m_canvas_is_dirty(true)
{
  // Menubar
  menubar = new FXMenuBar(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  statusbar = new FXStatusBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|FRAME_RAISED|STATUSBAR_WITH_DRAGCORNER);

  // fit menu
  fitmenu = new FXMenuPane(this);
  new FXMenuCommand(fitmenu,"&Run",NULL,this,ID_RUN_FIT);
  new FXMenuTitle(menubar,"&Fit",NULL,fitmenu);

  FXHorizontalFrame *mf = new FXHorizontalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXScrollWindow *iw = new FXScrollWindow(mf, VSCROLLER_ALWAYS | HSCROLLING_OFF | LAYOUT_FILL_Y);
  FXMatrix *matrix = new FXMatrix(iw, 2, LAYOUT_FILL_Y|MATRIX_BY_COLUMNS, 0, 0, 0, 0, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, 1, 1);

  fit_engine->config->subsampling = 0;

  fit_engine_prepare (fit_engine, spectrum, 0);

  m_fit_parameters = fit_parameters_new ();

  struct fit_parameters *params = fit_engine_get_all_parameters (fit_engine);

  m_parameters.resize(params->number);

  for (unsigned k = 0; k < m_parameters.size(); k++)
    {
      param_info& p = m_parameters[k];
      p.fp = params->values[k];

      double fpval = fit_engine_get_default_param_value (fit_engine, &p.fp);
      p.value = fpval;

      p.selected = false;
    }
  
  fit_parameters_free (params);

  Str pname;
  int current_layer = 0;
  for (unsigned k = 0; k < m_parameters.size(); k++)
    {
      fit_param_t *fp = &m_parameters[k].fp;

      if (fp->id == PID_LAYER_N && fp->layer_nb != current_layer)
	{
	  current_layer = fp->layer_nb;
	  FXString str;
	  str.format("Layer %i", current_layer);
	  new FXLabel(matrix, str);
	  new FXLabel(matrix, "");
	}

      get_param_name(fp, pname.str());
      FXString fxpname((const FXchar *) pname.cstr());
      FXCheckButton *bt = new FXCheckButton(matrix, fxpname, this, ID_PARAM_SELECT);
      FXTextField *tf = new FXTextField(matrix, 10, this, ID_PARAM_VALUE, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);

      param_info* p_inf = m_parameters.data() + k;

      tf->setUserData(p_inf);
      bt->setUserData(p_inf);

      p_inf->text_field = tf;
      
      FXString fptxt = FXStringFormat("%g", p_inf->value);
      tf->setText(fptxt, true);
    }

  canvas = new FXCanvas(mf, this, ID_CANVAS, LAYOUT_FILL_X|LAYOUT_FILL_Y);

  unsigned plot_mult = (fit_engine->system_kind == SYSTEM_REFLECTOMETER ? 1 : 2);
  m_plot.init(app, plot_mult);

  updatePlot(true);
}

interactive_fit::~interactive_fit() {
  fit_engine_disable(fit_engine);
  fit_engine_free(fit_engine);
  fit_parameters_free(m_fit_parameters);
  delete fitmenu;
}

long
interactive_fit::onCmdParamSelect(FXObject* _cb, FXSelector, void*)
{
  FXCheckButton *cb = (FXCheckButton *) _cb;
  param_info* p_inf = (param_info*) cb->getUserData();
  p_inf->selected = cb->getCheck();
  return 1;
}

void
interactive_fit::updatePlot(bool freeze_lmt)
{
  for (unsigned j = 0; j < m_parameters.size(); j++)
    {
      param_info& pi = m_parameters[j];
      fit_engine_apply_param (fit_engine, &pi.fp, pi.value);
    }

  switch (fit_engine->system_kind)
    {
    case SYSTEM_REFLECTOMETER:
      {
	plot *p = m_plot[0];
	refl_spectra_plot (fit_engine, p);
	if (freeze_lmt)
	  p->auto_limits(false);
	break;
      }
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
      {
	plot *p1 = m_plot[0], *p2 = m_plot[1];
	elliss_spectra_plot (fit_engine, p1, p2);
	if (freeze_lmt)
	  {
	    p1->auto_limits(false);
	    p2->auto_limits(false);
	  }
	break;
      }
    default:
      /* */ ;
    }
}

long
interactive_fit::onCmdParamChange(FXObject *_txt, FXSelector, void*)
{
  FXTextField *txt = (FXTextField *) _txt;
  FXString vstr = txt->getText();
  param_info* p_inf = (param_info*) txt->getUserData();
  double new_val = strtod (vstr.text(), NULL);

  if (new_val == p_inf->value)
    return 0;

  p_inf->value = new_val;
  m_canvas_is_dirty = true;
  return 1;
}

void
interactive_fit::drawPlot()
{
  FXDCWindow dc(canvas);
  int ww = canvas->getWidth(), hh = canvas->getHeight();
  draw (m_plot, &dc, ww, hh);
  m_canvas_is_dirty = false;
}

long
interactive_fit::onCmdPaint(FXObject*, FXSelector, void* ptr)
{
  drawPlot();
  return 1;
}


long
interactive_fit::onUpdCanvas(FXObject*, FXSelector, void* ptr)
{
  if (m_canvas_is_dirty)
    {
      updatePlot();
      drawPlot();
      return 1;
    }
  return 0;
}

long
interactive_fit::onCmdRunFit(FXObject*, FXSelector, void* ptr)
{
  reg_check_point(this);

  struct fit_parameters* fps = this->m_fit_parameters;

  fit_parameters_clear (fps);

  int fit_params_nb = 0;
  for (unsigned j = 0; j < m_parameters.size(); j++)
    {
      if (m_parameters[j].selected)
	fit_params_nb++;
    }

  gsl_vector* seeds = gsl_vector_alloc(fit_params_nb);

  int k = 0;
  for (unsigned j = 0; j < m_parameters.size(); j++)
    {
      if (m_parameters[j].selected)
	{
	  fit_parameters_add (fps, &m_parameters[j].fp);
	  gsl_vector_set (seeds, k, m_parameters[j].value);
	  k ++;
	}
    }

  fit_engine_set_parameters (fit_engine, fps);

  double chisq;
  lmfit_simple (fit_engine, seeds, &chisq, 0, 0, 0, 0, 0);

  k = 0;
  FXString ns;
  for (unsigned j = 0; j < m_parameters.size(); j++)
    {
      if (m_parameters[j].selected)
	{
	  double val = gsl_vector_get (fit_engine->results, k);
	  ns.format("%g", val);
	  m_parameters[j].text_field->setText(ns);
	  m_parameters[j].value = val;
	  k ++;
	}
    }

  gsl_vector_free (seeds);

  m_canvas_is_dirty = true;

  return 1;
}
