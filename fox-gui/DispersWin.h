#ifndef DISPERS_WIN_H
#define DISPERS_WIN_H

#include <fx.h>

#include "fx_plot.h"
#include "dispers.h"
#include "sampling.h"
#include "plot_canvas.h"

class DispersWin : public FXDialogBox {
  FXDECLARE(DispersWin)

public:
  DispersWin(FXWindow* w, disp_t *disp);

  ~DispersWin() { delete dispmenu; };

  void config_plot();

  long onCmdSetRange(FXObject*,FXSelector,void*);
  long onCmdSaveDisp(FXObject*,FXSelector,void*);

  enum {
    ID_SAVE_DISPERS = FXDialogBox::ID_LAST,
    ID_SPECTR_RANGE,
    ID_LAST
    };

protected:
  DispersWin(){};
private:
  DispersWin(const DispersWin&);
  DispersWin &operator=(const DispersWin&);

  disp_t* m_dispers;
  sampling_unif m_sampling;

  FXMenuPane* dispmenu;
  plot_canvas* m_canvas;
};

#endif
