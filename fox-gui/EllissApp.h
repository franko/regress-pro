#ifndef ELLISS_APP_H
#define ELLISS_APP_H

#include <fx.h>
#include "icon.h"

class EllissApp : public FXApp {
public:
  FXGIFIcon appicon;

  EllissApp() : 
    FXApp("Regress Pro", "Francesco Abbate"), 
    appicon(this, regressproicon) 
  { }
};

#endif
