
#include "InteractiveFit.h"
#include "Strcpp.h"
#include "fit-engine.h"

// Map
FXDEFMAP(InteractiveFit) InteractiveFitMap[]={
  FXMAPFUNC(SEL_COMMAND, InteractiveFit::ID_PARAM_SELECT, InteractiveFit::onCmdParamSelect),
  FXMAPFUNC(SEL_COMMAND, InteractiveFit::ID_PARAM_VALUE,  InteractiveFit::onCmdParamChange),
  FXMAPFUNC(SEL_CHANGED, InteractiveFit::ID_PARAM_VALUE,  InteractiveFit::onCmdParamChange),
};

// Object implementation
FXIMPLEMENT(InteractiveFit,FXDialogBox,InteractiveFitMap,ARRAYNUMBER(InteractiveFitMap));

InteractiveFit::InteractiveFit(FXWindow *w, struct symtab *s)
  : FXDialogBox(w, "Interactive Fit", DECOR_ALL, 0, 0, 640, 480),
    params(NULL)
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
  struct fit_engine *fit = build_fit_engine (s, &seeds);

  this->params = fit_engine_get_all_parameters (fit);

  Str pname;
  for (int k = 0; k < this->params->number; k++)
    {
      fit_param_t *fp = this->params->values + k;
      get_param_name(fp, pname.str());
      FXString fxpname((const FXchar *) pname.cstr());
      FXCheckButton *bt = new FXCheckButton(matrix, fxpname, this, ID_PARAM_SELECT);
      FXTextField *tf = new FXTextField(matrix, 10, this, ID_PARAM_VALUE, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
      double fpval = fit_engine_get_default_param_value (fit, fp);
      char fpvalbuf[16];
      int len = snprintf(fpvalbuf, 16, "%g", fpval);
      if (len >= 16)
	fpvalbuf[15] = 0;
      tf->setText(fpvalbuf, true);
    }

  //  FXCanvas *plotcanvas = new FXCanvas(mf, this, ID_CANVAS, LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(mf, "Canvas Placeholder!");

  fit_engine_free (fit);
}

InteractiveFit::~InteractiveFit() {
  if (this->params)
    fit_parameters_free(this->params);
  delete fitmenu;
}

long
InteractiveFit::onCmdParamSelect(FXObject*, FXSelector, void*)
{
  return 0;
}

long
InteractiveFit::onCmdParamChange(FXObject*, FXSelector, void*)
{
  return 0;
}
