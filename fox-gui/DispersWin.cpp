
#include "DispersWin.h"
#include "SpectrRangeWin.h"
#include "disp-util.h"
#include "disp_vs.h"

// Map
FXDEFMAP(DispersWin) DispersWinMap[]={
  FXMAPFUNC(SEL_COMMAND, DispersWin::ID_SPECTR_RANGE, DispersWin::onCmdSetRange),
  FXMAPFUNC(SEL_COMMAND, DispersWin::ID_SAVE_DISPERS, DispersWin::onCmdSaveDisp),
};

// Object implementation
FXIMPLEMENT(DispersWin,FXDialogBox,DispersWinMap,ARRAYNUMBER(DispersWinMap));

DispersWin::DispersWin(FXWindow* w, disp_t* disp)
  : FXDialogBox(w, "Dispersion Plot", DECOR_ALL, 0, 0, 480, 360),
    m_dispers(disp), m_sampling(240.0, 780.0, 271)
{

  FXMenuBar *menubar = new FXMenuBar(this,FRAME_RAISED|LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

  // Dispersion menu
  dispmenu = new FXMenuPane(this);
  new FXMenuCommand(dispmenu,"&Save Dispersion",NULL,this,ID_SAVE_DISPERS);
  new FXMenuCommand(dispmenu,"Spectral &Range",NULL,this,ID_SPECTR_RANGE);
  new FXMenuCommand(dispmenu,"&Close",NULL,this,ID_CANCEL);
  new FXMenuTitle(menubar,"&Dispersion",NULL,dispmenu);

  FXVerticalFrame *topfr = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

  m_canvas = new plot_canvas(topfr, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);

  new FXHorizontalSeparator(topfr,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  FXHorizontalFrame *buttonfr = new FXHorizontalFrame(topfr,LAYOUT_FILL_X|LAYOUT_RIGHT);
  new FXButton(buttonfr,"&Close",NULL,this,ID_CANCEL,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);

  config_plot();
}

void
DispersWin::config_plot()
{
  disp_t* d = m_dispers;
  sampling_unif& samp = m_sampling;

  disp_vs<sampling_unif>* disp_k = new disp_vs<sampling_unif>(d, cmpl::imag_part, samp);
  add_new_simple_plot(m_canvas, disp_k, "absoption coeff");

  disp_vs<sampling_unif>* disp_n = new disp_vs<sampling_unif>(d, cmpl::real_part, samp);
  add_new_simple_plot(m_canvas, disp_n, "refractive index");

  m_canvas->set_dirty(true);
}

long
DispersWin::onCmdSetRange(FXObject*,FXSelector,void*)
{
  SpectrRangeWin rangewin(this, &m_sampling);
  if (rangewin.execute())
    {
      m_canvas->update_limits();
      m_canvas->set_dirty(true);
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
      write_mat_file (fname.text(), m_dispers,
		      m_sampling.start(), m_sampling.end(), m_sampling.stride());
    }

  return 0;
}
