#ifndef DISPERS_WIN_H
#define DISPERS_WIN_H

#include <fx.h>

#include "dispers.h"
#include "sampling.h"
#include "plot_canvas.h"

class dispers_win : public FXDialogBox {
    FXDECLARE(dispers_win)

public:
    dispers_win(FXWindow* w, disp_t *disp);

    ~dispers_win() {
        delete dispmenu;
    };

    void config_plot();

    long on_cmd_set_range(FXObject*,FXSelector,void*);

    enum {
        ID_SPECTR_RANGE = FXDialogBox::ID_LAST,
        ID_LAST
    };

protected:
    dispers_win() {};
private:
    dispers_win(const dispers_win&);
    dispers_win &operator=(const dispers_win&);

    disp_t* m_dispers;
    sampling_unif m_sampling;

    FXMenuPane* dispmenu;
    plot_canvas* m_canvas;
};

#endif
