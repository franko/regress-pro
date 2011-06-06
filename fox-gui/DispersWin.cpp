
/*
  $Id: DispersWin.cpp,v 1.1 2006/07/12 22:57:48 francesco Exp $
 */

#include "DispersWin.h"
#include "SpectrRangeWin.h"
#include "disp-util.h"
#include "spectra-path.h"

// Map
FXDEFMAP(DispersWin) DispersWinMap[]={
  FXMAPFUNC(SEL_PAINT,   DispersWin::ID_CANVAS,       DispersWin::onCmdPaint),
  FXMAPFUNC(SEL_UPDATE,  DispersWin::ID_CANVAS,       DispersWin::onCmdUpdate),
  FXMAPFUNC(SEL_COMMAND, DispersWin::ID_SPECTR_RANGE, DispersWin::onCmdSetRange),
  FXMAPFUNC(SEL_COMMAND, DispersWin::ID_SAVE_DISPERS, DispersWin::onCmdSaveDisp),
};

// Object implementation
FXIMPLEMENT(DispersWin,FXDialogBox,DispersWinMap,ARRAYNUMBER(DispersWinMap));

DispersWin::DispersWin(FXWindow* w, disp_t *disp)
  : FXDialogBox(w, "Dispersion Plot", DECOR_ALL, 0, 0, 480, 360),
    dispers(disp)
{
  FXMenuBar *menubar = new FXMenuBar(this,FRAME_RAISED|LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

  // Dispersion menu
  dispmenu = new FXMenuPane(this);
  new FXMenuCommand(dispmenu,"&Save Dispersion",NULL,this,ID_SAVE_DISPERS);
  new FXMenuCommand(dispmenu,"Spectral &Range",NULL,this,ID_SPECTR_RANGE);
  new FXMenuCommand(dispmenu,"&Close",NULL,this,ID_CANCEL);
  new FXMenuTitle(menubar,"&Dispersion",NULL,dispmenu);

  FXVerticalFrame *topfr = new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  canvas = new FXCanvas(topfr,this,ID_CANVAS,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXHorizontalSeparator(topfr,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  FXHorizontalFrame *buttonfr = new FXHorizontalFrame(topfr,LAYOUT_FILL_X|LAYOUT_RIGHT);
  new FXButton(buttonfr,"&Close",NULL,this,ID_CANCEL,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);

  const double wlinf = 200, wlsup = 800, wlstep = 2;
  range = SpectrRange(wlinf, wlsup, wlstep);

  nplot = new plot(getApp());
  kplot = new plot(getApp());

  nplot->set_title("Refractive index, n");
  kplot->set_title("Absorption coeff, k");

  setupNKPlot();
}

DispersWin::~DispersWin()
{
  delete dispmenu;
  delete nplot;
  delete kplot;
}

void
DispersWin::setupNKPlot()
{
  agg::path_storage *n_path = new agg::path_storage;
  agg::path_storage *k_path = new agg::path_storage;

  int npt = (int) ((range.sup - range.inf) / range.step) + 1;
  
  for (int j = 0; j < npt; j++)
    {
      double lambda = range.inf + range.step * j;
      double n, k;
  
      n_value_cpp (dispers, lambda, &n, &k);

      n_path->line_to(lambda, n);
      k_path->line_to(lambda, k);
    }

  nplot->clear();
  nplot->add(n_path);

  kplot->clear();
  kplot->add(k_path);
  
  plotNeedRedraw = TRUE;
}

long
DispersWin::onCmdUpdate(FXObject *, FXSelector, void *)
{
  if (plotNeedRedraw == FALSE)
    return 0;

  FXDCWindow dc(canvas);

  int ww = canvas->getWidth(), hh = canvas->getHeight();

  nplot->draw(&dc, ww, hh / 2);
  kplot->draw(&dc, ww, hh / 2, 0, hh / 2);

  plotNeedRedraw = FALSE;

  return 1;
}

long
DispersWin::onCmdPaint(FXObject *, FXSelector, void *ptr)
{
  FXDCWindow dc(canvas, (FXEvent*) ptr);

  int ww = canvas->getWidth(), hh = canvas->getHeight();

  nplot->draw(&dc, ww, hh / 2);
  kplot->draw(&dc, ww, hh / 2, 0, hh / 2);

  return 1;
}

long
DispersWin::onCmdSetRange(FXObject*,FXSelector,void*)
{
  SpectrRangeWin rangewin(this, range);
  if (rangewin.execute())
    {
      rangewin.getSpectrRange(this->range);
      setupNKPlot();
      return 1;
    }
  return 0;
}

long
DispersWin::onCmdSaveDisp(FXObject*,FXSelector,void*)
{
  FXFileDialog save(this, "Save Dispersion Card");
  save.setFilename("untitled.mat");

  if(save.execute())
    {
      FXString fname = save.getFilename();
      write_mat_file (fname.text(), this->dispers,
		      range.inf, range.sup, range.step);
    }

  return 0;
}
