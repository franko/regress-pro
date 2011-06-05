/* -*- mode: C++; -*- */

#ifndef INTERATIVE_FIT_H
#define INTERATIVE_FIT_H

#include <fx.h>

#include "symtab.h"
#include "fit-params.h"

class InteractiveFit : public FXDialogBox {
  FXDECLARE(InteractiveFit)

private:
  struct fit_parameters *params;

protected:
  FXMenuBar         *menubar;
  FXStatusBar       *statusbar;
  FXMenuPane        *fitmenu;

protected:
  InteractiveFit(){};
private:
  InteractiveFit(const InteractiveFit&);
  InteractiveFit &operator=(const InteractiveFit&);

public:
  InteractiveFit(FXWindow *w, struct symtab *symtab);
  virtual ~InteractiveFit();

  long onCmdParamSelect(FXObject*,FXSelector,void*);
  long onCmdParamChange(FXObject*,FXSelector,void*);

  enum {
    ID_PARAM_SELECT = FXDialogBox::ID_LAST,
    ID_PARAM_VALUE,
    ID_RUN_FIT,
    ID_CANVAS,
    ID_LAST
  };
};

#endif
