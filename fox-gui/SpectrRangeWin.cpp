
/*
  $Id: SpectrRangeWin.cpp,v 1.1 2006/07/12 22:57:48 francesco Exp $
 */

#include "SpectrRangeWin.h"

// Map
FXDEFMAP(SpectrRangeWin) SpectrRangeWinMap[]={
  FXMAPFUNC(SEL_CHANGED, SpectrRangeWin::ID_LOWER_SET, SpectrRangeWin::onCmdLmtChange),
  FXMAPFUNC(SEL_CHANGED, SpectrRangeWin::ID_UPPER_SET, SpectrRangeWin::onCmdLmtChange),
  FXMAPFUNC(SEL_CHANGED, SpectrRangeWin::ID_STEP_SET,  SpectrRangeWin::onCmdLmtChange),
};

// Object implementation
FXIMPLEMENT(SpectrRangeWin,FXDialogBox,SpectrRangeWinMap,ARRAYNUMBER(SpectrRangeWinMap));

SpectrRangeWin::SpectrRangeWin(FXWindow *win, SpectrRange &r)
  : FXDialogBox(win, "Select Spectral Range")
{
  FXVerticalFrame *mfr  = new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXMatrix *matrix = new FXMatrix(mfr,3,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(matrix, "Lower limit", NULL, LAYOUT_FILL_ROW);
  new FXLabel(matrix, "Upper limit", NULL, LAYOUT_FILL_ROW);
  new FXLabel(matrix, "Sample step", NULL, LAYOUT_FILL_ROW);

  FXTextField *tf = new FXTextField(matrix,10,this,ID_LOWER_SET,FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
  tf->setText(FXStringVal(r.inf, 1, FALSE));

  tf = new FXTextField(matrix,10,this,ID_UPPER_SET,FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
  tf->setText(FXStringVal(r.sup, 1, FALSE));

  tf = new FXTextField(matrix,10,this,ID_STEP_SET,FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
  tf->setText(FXStringVal(r.step, 1, FALSE));

  new FXHorizontalSeparator(mfr,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  FXHorizontalFrame *btframe = new FXHorizontalFrame(mfr,LAYOUT_FILL_X|LAYOUT_RIGHT);
  new FXButton(btframe,"&Cancel",NULL,this,ID_CANCEL,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);
  new FXButton(btframe,"&Ok",NULL,this,ID_ACCEPT,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);

  range = r;
}

long
SpectrRangeWin::onCmdLmtChange(FXObject* sender,FXSelector sel,void*)
{
  FXTextField *tf = (FXTextField *) sender;
  double val = FXDoubleVal(tf->getText());
  FXint id = FXSELID(sel);

  switch (id)
    {
    case ID_LOWER_SET:
      range.inf = val;
      break;
    case ID_UPPER_SET:
      range.sup = val;
      break;
    case ID_STEP_SET:
      range.step = val;
    default:
      /* */;
    }

  return 0;
}

