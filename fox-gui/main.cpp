
/*
  $Id: main.cpp,v 1.2 2006/12/29 17:47:08 francesco Exp $
 */

#include "EllissGui.h"

int main(int argc,char *argv[]){

  // Make application
  EllissApp app;

  // Open display
  app.init(argc, argv);

  // Main window
  EllissWindow* window = new EllissWindow(&app);

  // Create app
  app.create();

  // Show it
  window->show(PLACEMENT_SCREEN);

  // Run
  return app.run();
}
