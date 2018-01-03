
#include "ProgressInfo.h"

ProgressInfo::ProgressInfo(FXApp *a, FXWindow *w)
    : app(a), window(w), dialog(nullptr)
{
    gettimeofday(this->start, nullptr);
}

ProgressInfo::~ProgressInfo()
{
    if(dialog == nullptr) {
        return;
    }

    delete dialog;
}

void
ProgressInfo::hide()
{
    if(dialog == nullptr) {
        return;
    }

    dialog->hide();
    app->endWaitCursor();
}
