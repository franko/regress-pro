
#include "InteractiveFit.h"
#include "Strcpp.h"
#include "fit-engine.h"
#include "spectra-path.h"

// Map
FXDEFMAP(InteractiveFit) InteractiveFitMap[]={
  FXMAPFUNC(SEL_COMMAND, InteractiveFit::ID_PARAM_SELECT, InteractiveFit::onCmdParamSelect),
  FXMAPFUNC(SEL_COMMAND, InteractiveFit::ID_PARAM_VALUE,  InteractiveFit::onCmdParamChange),
  FXMAPFUNC(SEL_CHANGED, InteractiveFit::ID_PARAM_VALUE,  InteractiveFit::onCmdParamChange),
  FXMAPFUNC(SEL_PAINT,   InteractiveFit::ID_CANVAS,       InteractiveFit::onCmdPaint),
  //  FXMAPFUNC(SEL_UPDATE,  InteractiveFit::ID_CANVAS,       InteractiveFit::onCmdPaint),
};

// Object implementation
FXIMPLEMENT(InteractiveFit,FXMainWindow,InteractiveFitMap,ARRAYNUMBER(InteractiveFitMap));

InteractiveFit::InteractiveFit(FXApp *app, struct symtab *s, struct spectrum *user_spectr)
  : FXMainWindow(app, "Interactive Fit", NULL, NULL, DECOR_ALL, 0, 0, 640, 480),
    params(NULL), fit_engine(NULL), spectrum(user_spectr)
{
  // Menubar
  menubar = new FXMenuBar(this, FRAME_RAISED|LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  statusbar = new FXStatusBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|FRAME_RAISED|STATUSBAR_WITH_DRAGCORNER);

  // fit menu
  fitmenu = new FXMenuPane(this);
  new FXMenuCommand(fitmenu,"&Running",NULL,this,ID_RUN_FIT);
  new FXMenuTitle(menubar,"&Fit",NULL,fitmenu);

  FXHorizontalFrame *mf = new FXHorizontalFrame(this, FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXMatrix *matrix = new FXMatrix(mf, 2, LAYOUT_FILL_Y|MATRIX_BY_COLUMNS, 0, 0, 0, 0, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, 1, 1);

  struct seeds *seeds;
  fit_engine = build_fit_engine (s, &seeds);
  fit_engine->config->subsampling = 0;

  fit_engine_prepare (fit_engine, spectrum);

  params = fit_engine_get_all_parameters (fit_engine);

  param_values = gsl_vector_alloc (params->number);
  
  Str pname;
  for (int k = 0; k < params->number; k++)
    {
      fit_param_t *fp = this->params->values + k;
      get_param_name(fp, pname.str());
      FXString fxpname((const FXchar *) pname.cstr());
      FXCheckButton *bt = new FXCheckButton(matrix, fxpname, this, ID_PARAM_SELECT);
      FXTextField *tf = new FXTextField(matrix, 10, this, ID_PARAM_VALUE, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
      tf->setUserData((void *) (&param_values->data[k]));

      double fpval = fit_engine_get_default_param_value (fit_engine, fp);
      gsl_vector_set (param_values, k, fpval);
      
      char fpvalbuf[16];
      int len = snprintf(fpvalbuf, 16, "%g", fpval);
      if (len >= 16)
	fpvalbuf[15] = 0;
      tf->setText(fpvalbuf, true);
    }

  canvas = new FXCanvas(mf, this, ID_CANVAS, LAYOUT_FILL_X|LAYOUT_FILL_Y);

  unsigned plot_mult = (fit_engine->system_kind == SYSTEM_REFLECTOMETER ? 1 : 2);
  m_plots.init(app, plot_mult);

  updatePlot();
}

InteractiveFit::~InteractiveFit() {
  if (params)
    fit_parameters_free(params);

  fit_engine_disable(fit_engine);
  fit_engine_free(fit_engine);

  gsl_vector_free (param_values);

  delete fitmenu;
}

long
InteractiveFit::onCmdParamSelect(FXObject*, FXSelector, void*)
{
  return 0;
}

void
InteractiveFit::updatePlot()
{
  switch (fit_engine->system_kind)
    {
    case SYSTEM_REFLECTOMETER:
      {
	plot *p = m_plots[0];
	refl_spectra_plot (fit_engine, p);
	break;
      }
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
      {
	plot *p1 = m_plots[0], *p2 = m_plots[1];
	elliss_spectra_plot (fit_engine, p1, p2);
	break;
      }
    default:
      /* */ ;
    }
}

long
InteractiveFit::onCmdParamChange(FXObject *_txt, FXSelector, void*)
{
  if (m_plots.size() > 0)
    {
      FXTextField *txt = (FXTextField *) _txt;
      FXString vstr = txt->getText();
      double * paddr = (double *) txt->getUserData();
      double new_val = strtod (vstr.text(), NULL);

      if (new_val == *paddr)
	return 0;

      *paddr = new_val;

      fit_engine_apply_parameters (fit_engine, params, param_values);

      updatePlot();
      drawPlot();

      return 1;
    }

  return 0;
}

void
InteractiveFit::drawPlot()
{
  FXDCWindow dc(canvas);
  int ww = canvas->getWidth(), hh = canvas->getHeight();
  int n = m_plots.size();
  for (unsigned j = 0; j < n; j++)
    m_plots[j]->draw(&dc, ww, hh/n, 0, hh*j/n);
}

long
InteractiveFit::onCmdPaint(FXObject*, FXSelector, void* ptr)
{
  drawPlot();
  return 1;
}
