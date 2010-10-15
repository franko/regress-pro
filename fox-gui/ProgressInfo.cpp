
#include "ProgressInfo.h"

ProgressInfo::ProgressInfo(FXApp *a, FXWindow *w)
  : app(a), window(w), dialog(NULL)
{
  gettimeofday (this->start, NULL);
}

ProgressInfo::~ProgressInfo()
{
  if (dialog == NULL)
    return;

  delete dialog;
}

void
ProgressInfo::hide()
{
  if (dialog == NULL)
    return;

  dialog->hide();
  app->endWaitCursor();
}
