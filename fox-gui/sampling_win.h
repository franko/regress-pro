
#ifndef SPECTR_RANGE_H
#define SPECTR_RANGE_H

#include <fx.h>
#include "sampling.h"

class sampling_win : public FXDialogBox {
    FXDECLARE(sampling_win)

private:
    sampling_unif* m_sampling;

    FXTextField *m_start;
    FXTextField *m_end;
    FXTextField *m_stride;

protected:
    sampling_win() {};
private:
    sampling_win(const sampling_win&);
    sampling_win &operator=(const sampling_win&);

public:
    sampling_win(FXWindow *win, sampling_unif* s);

    long on_cmd_accept(FXObject*,FXSelector,void*);
};

#endif
