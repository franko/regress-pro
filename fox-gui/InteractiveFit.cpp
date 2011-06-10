
#include "InteractiveFit.h"
#include "Strcpp.h"
#include "fit-engine.h"
#include "lmfit-simple.h"
#include "spectra-path.h"

// Map
FXDEFMAP(InteractiveFit) InteractiveFitMap[]={
  FXMAPFUNC(SEL_COMMAND, InteractiveFit::ID_PARAM_SELECT, InteractiveFit::onCmdParamSelect),
  FXMAPFUNC(SEL_COMMAND, InteractiveFit::ID_PARAM_VALUE,  InteractiveFit::onCmdParamChange),
  FXMAPFUNC(SEL_CHANGED, InteractiveFit::ID_PARAM_VALUE,  InteractiveFit::onCmdParamChange),
  FXMAPFUNC(SEL_PAINT,   InteractiveFit::ID_CANVAS,       InteractiveFit::onCmdPaint),
  FXMAPFUNC(SEL_UPDATE,  InteractiveFit::ID_CANVAS,       InteractiveFit::onUpdCanvas),
  FXMAPFUNC(SEL_COMMAND, InteractiveFit::ID_RUN_FIT,      InteractiveFit::onCmdRunFit),
};

// Object implementation
FXIMPLEMENT(InteractiveFit,FXMainWindow,InteractiveFitMap,ARRAYNUMBER(InteractiveFitMap));

InteractiveFit::InteractiveFit(FXApp *app, struct symtab *s, struct spectrum *user_spectr)
  : FXMainWindow(app, "Interactive Fit", NULL, NULL, DECOR_ALL, 0, 0, 640, 480),
    fit_engine(NULL), spectrum(user_spectr), m_canvas_is_dirty(true)
{
  // Menubar
  menubar = new FXMenuBar(this, FRAME_RAISED|LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  statusbar = new FXStatusBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|FRAME_RAISED|STATUSBAR_WITH_DRAGCORNER);

  // fit menu
  fitmenu = new FXMenuPane(this);
  new FXMenuCommand(fitmenu,"&Run",NULL,this,ID_RUN_FIT);
  new FXMenuTitle(menubar,"&Fit",NULL,fitmenu);

  FXHorizontalFrame *mf = new FXHorizontalFrame(this, FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXMatrix *matrix = new FXMatrix(mf, 2, LAYOUT_FILL_Y|MATRIX_BY_COLUMNS, 0, 0, 0, 0, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, 1, 1);

  struct seeds *seeds;
  fit_engine = build_fit_engine (s, &seeds);
  fit_engine->config->subsampling = 0;

  fit_engine_prepare (fit_engine, spectrum, 0);

  struct fit_parameters *params = fit_engine_get_all_parameters (fit_engine);
  m_parameters.init(params);

  m_params_text_field.resize(m_parameters.number);

  for (int k = 0; k < m_parameters.number; k++)
    {
      fit_param_t *fp = params->values + k;
      double fpval = fit_engine_get_default_param_value (fit_engine, fp);
      gsl_vector_set (m_parameters.values, k, fpval);
    }

  Str pname;
  for (int k = 0; k < m_parameters.number; k++)
    {
      fit_param_t *fp = params->values + k;
      get_param_name(fp, pname.str());
      FXString fxpname((const FXchar *) pname.cstr());
      FXCheckButton *bt = new FXCheckButton(matrix, fxpname, this, ID_PARAM_SELECT);
      FXTextField *tf = new FXTextField(matrix, 10, this, ID_PARAM_VALUE, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);

      void *offset_ptr = (void *) (m_parameters.base_ptr + k);
      tf->setUserData(offset_ptr);
      bt->setUserData(offset_ptr);

      m_params_text_field[k] = tf;
      
      double fpval = gsl_vector_get (m_parameters.values, k);
      char fpvalbuf[16];
      int len = snprintf(fpvalbuf, 16, "%g", fpval);
      if (len >= 16)
	fpvalbuf[15] = 0;
      tf->setText(fpvalbuf, true);
    }

  canvas = new FXCanvas(mf, this, ID_CANVAS, LAYOUT_FILL_X|LAYOUT_FILL_Y);

  unsigned plot_mult = (fit_engine->system_kind == SYSTEM_REFLECTOMETER ? 1 : 2);
  m_plots.init(app, plot_mult);

  updatePlot(true);
}

InteractiveFit::~InteractiveFit() {
  fit_engine_disable(fit_engine);
  fit_engine_free(fit_engine);
  delete fitmenu;
}

long
InteractiveFit::onCmdParamSelect(FXObject* _cb, FXSelector, void*)
{
  FXCheckButton *cb = (FXCheckButton *) _cb;
  double * paddr = (double *) cb->getUserData();
  int k = (paddr - m_parameters.base_ptr);
  m_parameters.select[k] = cb->getCheck();
  return 1;
}

void
InteractiveFit::updatePlot(bool freeze_lmt)
{
  struct fit_parameters* ps = m_parameters.parameters;
  fit_engine_apply_parameters (fit_engine, ps, m_parameters.values);

  switch (fit_engine->system_kind)
    {
    case SYSTEM_REFLECTOMETER:
      {
	plot *p = m_plots[0];
	refl_spectra_plot (fit_engine, p);
	if (freeze_lmt)
	  p->auto_limits(false);
	break;
      }
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
      {
	plot *p1 = m_plots[0], *p2 = m_plots[1];
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
InteractiveFit::onCmdParamChange(FXObject *_txt, FXSelector, void*)
{
  FXTextField *txt = (FXTextField *) _txt;
  FXString vstr = txt->getText();
  double * paddr = (double *) txt->getUserData();
  double new_val = strtod (vstr.text(), NULL);

  if (new_val == *paddr)
    return 0;

  *paddr = new_val;
  m_canvas_is_dirty = true;
  return 1;
}

void
InteractiveFit::drawPlot()
{
  FXDCWindow dc(canvas);
  int ww = canvas->getWidth(), hh = canvas->getHeight();
  int n = m_plots.size();
  for (unsigned j = 0; j < n; j++)
    m_plots[j]->draw(&dc, ww, hh/n, 0, hh*j/n);

  m_canvas_is_dirty = false;
}

long
InteractiveFit::onCmdPaint(FXObject*, FXSelector, void* ptr)
{
  drawPlot();
  return 1;
}


long
InteractiveFit::onUpdCanvas(FXObject*, FXSelector, void* ptr)
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
InteractiveFit::onCmdRunFit(FXObject*, FXSelector, void* ptr)
{
  struct fit_parameters* fps = fit_parameters_new ();

  int fit_params_nb = 0;
  for (int j = 0; j < m_parameters.number; j++)
    {
      if (m_parameters.select[j])
	fit_params_nb++;
    }

  gsl_vector* seeds = gsl_vector_alloc(fit_params_nb);

  int k = 0;
  for (int j = 0; j < m_parameters.number; j++)
    {
      if (m_parameters.select[j])
	{
	  fit_parameters_add (fps, m_parameters.parameters->values + j);
	  gsl_vector_set (seeds, k, gsl_vector_get (m_parameters.values, j));
	  k ++;
	}
    }

  fit_engine_set_parameters (fit_engine, fps);

  double chisq;
  lmfit_simple (fit_engine, seeds, &chisq, 0, 0, 0, 0, 0);

  k = 0;
  FXString ns;
  for (int j = 0; j < m_parameters.number; j++)
    {
      if (m_parameters.select[j])
	{
	  double val = gsl_vector_get(fit_engine->results, k);
	  ns.format("%g", val);
	  m_params_text_field[j]->setText(ns);
	  gsl_vector_set(m_parameters.values, j, val);
	  k ++;
	}
    }

  m_canvas_is_dirty = true;

  return 1;
}
