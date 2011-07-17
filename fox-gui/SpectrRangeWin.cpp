
#include "SpectrRangeWin.h"

// Map
FXDEFMAP(SpectrRangeWin) SpectrRangeWinMap[]={
  FXMAPFUNC(SEL_COMMAND, FXDialogBox::ID_ACCEPT, SpectrRangeWin::on_cmd_accept),
};

// Object implementation
FXIMPLEMENT(SpectrRangeWin,FXDialogBox,SpectrRangeWinMap,ARRAYNUMBER(SpectrRangeWinMap));

SpectrRangeWin::SpectrRangeWin(FXWindow *win, sampling_unif* samp)
  : FXDialogBox(win, "Select Spectral Range"), m_sampling(samp)
{
  FXVerticalFrame *mfr  = new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXMatrix *matrix = new FXMatrix(mfr,3,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(matrix, "Lower limit", NULL, LAYOUT_FILL_ROW);
  new FXLabel(matrix, "Upper limit", NULL, LAYOUT_FILL_ROW);
  new FXLabel(matrix, "Sample step", NULL, LAYOUT_FILL_ROW);

  FXString txt;

  m_start  = new FXTextField(matrix, 10, NULL, 0, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
  txt.format("%g", m_sampling->start());
  m_start->setText(txt);

  m_end    = new FXTextField(matrix, 10, NULL, 0, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
  txt.format("%g", m_sampling->end());
  m_end->setText(txt);

  m_stride = new FXTextField(matrix, 10, NULL, 0, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
  txt.format("%g", m_sampling->stride());
  m_stride->setText(txt);

  new FXHorizontalSeparator(mfr,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  FXHorizontalFrame *btframe = new FXHorizontalFrame(mfr,LAYOUT_FILL_X|LAYOUT_RIGHT);
  new FXButton(btframe,"&Cancel",NULL,this,ID_CANCEL,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);
  new FXButton(btframe,"&Ok",NULL,this,ID_ACCEPT,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);
}

long
SpectrRangeWin::on_cmd_accept(FXObject* sender,FXSelector sel,void*)
{
  double wls, wle, wlp;
  char* tail;

  FXString txt = m_start->getText();
  wls = strtod (txt.text(), &tail);

  txt = m_end->getText();
  wle = strtod (txt.text(), &tail);

  txt = m_stride->getText();
  wlp = strtod (txt.text(), &tail);

  if (wlp != 0)
    {
      int npt = (wle - wls) / wlp + 1;

      if (npt > 8 && npt < 1024)
	{
	  m_sampling->set(wls, wle, npt);

	  getApp()->stopModal(this, TRUE);
	  hide();
	}
    }

  return 1;
}
