/******************************************************************************
*                                                                             *
*                                  Elliss Gui                                 *
*                                                                             *
*******************************************************************************
* Copyright (C) 2005,2006 by Francesco Abbate.   All Rights Reserved.         *
*******************************************************************************
* $Id: EllissGui.h,v 1.4 2006/12/29 17:47:08 francesco Exp $             *
******************************************************************************/

#include <fx.h>
#include <sys/time.h>

#include "ProgressInfo.h"

#include "str.h"

#include "fxscale.h"
#include "spectra.h"
#include "symtab.h"

class EllissWindow : public FXMainWindow {
  FXDECLARE(EllissWindow)

protected:
  struct spectrum *spectrum;
  struct stack *stack_result;
  struct symtab symtab[1];

  FXString scriptFile;
  FXString spectrFile;
  FXString batchFileId;

protected:
  FXMenuBar         *menubar;
  FXStatusBar       *statusbar;

  FXMenuPane        *filemenu;
  FXMenuPane        *spectrmenu;
  FXMenuPane        *dispmenu;
  FXMenuPane        *fitmenu;
  FXMenuPane        *helpmenu;

  FXTabBook         *tabbook;
  FXTabItem         *tabscript, *tabplot;
  FXText            *scripttext;
  FXText            *resulttext;
  FXCanvas          *plotcanvas;

  FXFont            *scriptfont;

  static const FXchar patterns_fit[];
  static const FXchar patterns_spectr[];

  static const FXHiliteStyle tstyles[];
  
  bool isPlotModified;

  enum system_kind plotkind;
  FXDataPlot *spectrPlot1;
  FXDataPlot *spectrPlot2;

protected:
  EllissWindow(){};
private:
  EllissWindow(const EllissWindow&);
  EllissWindow &operator=(const EllissWindow&);
public:
  long onCmdPaint(FXObject*,FXSelector,void*);
  long onUpdCanvas(FXObject*,FXSelector,void*);
  long onCmdLoadScript(FXObject*,FXSelector,void*);
  long onCmdSaveScript(FXObject*,FXSelector,void*);
  long onCmdSaveAsScript(FXObject*,FXSelector,void *);
  long onCmdLoadSpectra(FXObject*,FXSelector,void*);
  long onCmdPlotDispers(FXObject*,FXSelector,void*);
  long onCmdRunFit(FXObject*,FXSelector,void*);
  long onCmdRunMultiFit(FXObject*,FXSelector,void*);
  long onCmdRunSimul(FXObject*,FXSelector,void*);
  long onCmdRunBatch(FXObject*,FXSelector,void*);
  long onCmdAbout(FXObject*,FXSelector,void*);

  FXbool loadfile(const FXString& file, str_t buffer);
  FXbool setFitStrategy(const char *script_text);
  FXbool saveScriptAs (const FXString& save_as);
  void updateFitStrategy();
  void setErrorRegion (int sl, int el);
  void cleanScriptErrors ();
  void plotCanvas(FXDCWindow *dc);
  void reportErrors();
public:
  enum {
    ID_CANVAS = FXMainWindow::ID_LAST,
    ID_LOAD_SCRIPT,
    ID_SAVE_SCRIPT,
    ID_SAVEAS_SCRIPT,
    ID_LOAD_SPECTRA,
    ID_DISP_PLOT,
    ID_RUN_FIT,
    ID_RUN_MULTI_FIT,
    ID_RUN_BATCH,
    ID_RUN_SIMUL,
    ID_ABOUT,
    ID_LAST
    };
public:
  EllissWindow(FXApp *a);
  virtual void create();
  virtual ~EllissWindow();
};

extern "C" {

  extern int process_foxgui_events (void *data, float p, const char *msg);

};
