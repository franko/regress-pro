/*
  $Id: BatchDialog.h,v 1.2 2006/12/26 18:16:01 francesco Exp $
 */

#ifndef BATCH_DIALOG_H
#define BATCH_DIALOG_H

#include <fx.h>

#include "fit-engine.h"
#include "ProgressInfo.h"

class BatchDialog : public FXDialogBox {
  FXDECLARE(BatchDialog)
private:
  FXTextField *tfName;
  FXTextField *tfStart;
  FXTextField *tfEnd;
  FXTextField *tfStep;

  struct fit_engine *fit;
  struct seeds *seeds;
  FXString result;

protected:
  BatchDialog(){};
private:
  BatchDialog(const BatchDialog&);
  BatchDialog &operator=(const BatchDialog&);

public:
  BatchDialog(FXWindow *w, struct fit_engine *fit, struct seeds *seeds);
  void execute(FXString &res);

  long onCmdRun(FXObject* sender,FXSelector,void*);
  long onCmdBrowse(FXObject* sender,FXSelector,void*);

  inline void setFilename(const FXString &fn) { tfName->setText(fn); };
  inline FXString getFilename() { return tfName->getText(); };

  enum {
    ID_BROWSE = FXDialogBox::ID_LAST,
    ID_LAST
  };
};


#endif
