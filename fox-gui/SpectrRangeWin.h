
#ifndef SPECTR_RANGE_H
#define SPECTR_RANGE_H

#include <fx.h>
#include "sampling.h"

class SpectrRangeWin : public FXDialogBox {
  FXDECLARE(SpectrRangeWin)

private:
  sampling_unif* m_sampling;

  FXTextField *m_start;
  FXTextField *m_end;
  FXTextField *m_stride;

protected:
  SpectrRangeWin() {};
private:
  SpectrRangeWin(const SpectrRangeWin&);
  SpectrRangeWin &operator=(const SpectrRangeWin&);

public:
  SpectrRangeWin(FXWindow *win, sampling_unif* s);

  long on_cmd_accept(FXObject*,FXSelector,void*);
};

#endif
