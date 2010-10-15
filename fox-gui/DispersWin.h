
/*
  $Id: DispersWin.h,v 1.1 2006/07/12 22:57:48 francesco Exp $
 */

#ifndef DISPERS_WIN_H
#define DISPERS_WIN_H

#include <fx.h>

#include "fxscale.h"
#include "dispers.h"

struct SpectrRange {
  float inf, sup;
  float step;

  SpectrRange() : inf(200.0), sup(800.0), step(5.0) {};
  SpectrRange(double ginf, double gsup, double gstep = 1.0) :
    inf(ginf), sup(gsup), step(gstep) {};
};

class DispersWin : public FXDialogBox {
  FXDECLARE(DispersWin)
private:
  FXDataPlot *nplot;
  FXDataPlot *kplot;

  FXbool plotNeedRedraw;

  FXMenuPane *dispmenu;
  FXCanvas *canvas;

  disp_t *dispers;
  SpectrRange range;

protected:
  DispersWin(){};
private:
  DispersWin(const DispersWin&);
  DispersWin &operator=(const DispersWin&);
public:
  DispersWin(FXWindow* w, disp_t *disp);
  ~DispersWin();

  void setupNKPlot();

public:
  long onCmdPaint(FXObject*,FXSelector,void*);
  long onCmdUpdate(FXObject*,FXSelector,void*);
  long onCmdSetRange(FXObject*,FXSelector,void*);
  long onCmdSaveDisp(FXObject*,FXSelector,void*);

  enum {
    ID_CANVAS = FXDialogBox::ID_LAST,
    ID_SAVE_DISPERS,
    ID_SPECTR_RANGE,
    ID_LAST
    };
};


#endif
