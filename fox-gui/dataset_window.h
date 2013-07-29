#ifndef FOXGUI_DATASET_WINDOW_H
#define FOXGUI_DATASET_WINDOW_H

#include "fx.h"

class dataset_window : public FXMainWindow {
  FXDECLARE(dataset_window)
protected:

  // Member data
  FXMenuBar*         menubar;
  FXMenuPane*        filemenu;
  FXMenuPane*        manipmenu;
  FXTable*           table;

protected:
  dataset_window(){}

public:
  dataset_window(FXApp* a,const FXString& name,FXIcon *ic=NULL,FXIcon *mi=NULL,FXuint opts=DECOR_ALL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,FXint hs=0,FXint vs=0);

//  long on_cmd_close(FXObject*,FXSelector,void *);

public:
  enum{
    ID_TEST=FXMainWindow::ID_LAST,
    ID_LOAD_FILE,
    ID_LOAD_FILES,
    ID_LAST
  };

public:
  virtual ~dataset_window();
  virtual FXbool close(FXbool notify);
};

#endif
