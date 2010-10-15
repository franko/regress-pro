/* -*- mode: C++; -*- */

/*
  $Id: DispersDialog.h,v 1.1 2006/07/12 22:57:48 francesco Exp $
 */

#ifndef DISPERS_DIALOG_H
#define DISPERS_DIALOG_H

#include <fx.h>

#include "stack.h"

class DispersDialog : public FXDialogBox {
  FXDECLARE(DispersDialog)
private:
  struct stack *stack;
  int layer;

  FXLabel *dispLabel;
  FXSpinner *layerSpinner;

protected:
  DispersDialog(){};
private:
  DispersDialog(const DispersDialog&);
  DispersDialog &operator=(const DispersDialog&);

public:
  DispersDialog(FXWindow *w, struct stack *s);

  long onCmdLayerChange(FXObject*,FXSelector,void*);
  long onCmdPlot(FXObject*,FXSelector,void*);

  enum {
    ID_LAYER_CHANGE = FXDialogBox::ID_LAST,
    ID_PLOT,
    ID_LAST
  };
};

#endif
