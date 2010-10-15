
/*
  $Id: SpectrRangeWin.h,v 1.1 2006/07/12 22:57:48 francesco Exp $
 */

#ifndef SPECTR_RANGE_H
#define SPECTR_RANGE_H

#include <fx.h>
#include "DispersWin.h"

class SpectrRangeWin : public FXDialogBox {
  FXDECLARE(SpectrRangeWin)

private:
  SpectrRange range;

protected:
  SpectrRangeWin(){};
private:
  SpectrRangeWin(const SpectrRangeWin&);
  SpectrRangeWin &operator=(const SpectrRangeWin&);

public:
  SpectrRangeWin(FXWindow *win, SpectrRange &r);
  void getSpectrRange(SpectrRange &r) const { r = range; }

  long onCmdLmtChange(FXObject*,FXSelector,void*);

  enum {
    ID_LOWER_SET = FXDialogBox::ID_LAST,
    ID_UPPER_SET,
    ID_STEP_SET,
    ID_LAST
  };
};

#endif
