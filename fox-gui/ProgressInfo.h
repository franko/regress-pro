/*
  $Id$
*/

#ifndef PROGRESS_INFO_H
#define PROGRESS_INFO_H

#include <fx.h>
#include <sys/time.h>

struct ProgressInfo {
    FXApp *app;
    FXWindow *window;
    FXProgressDialog *dialog;
    struct timeval start[1];

public:
    ProgressInfo(FXApp *a, FXWindow *w);
    ~ProgressInfo();

    void hide();
};

#endif
