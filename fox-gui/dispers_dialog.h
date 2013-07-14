/* -*- mode: C++; -*- */

#ifndef DISPERS_DIALOG_H
#define DISPERS_DIALOG_H

#include <fx.h>

#include "stack.h"

class dispers_dialog : public FXDialogBox {
    FXDECLARE(dispers_dialog)
private:
    struct stack *stack;
    int layer;

    FXLabel *dispLabel;
    FXSpinner *layerSpinner;

protected:
    dispers_dialog() {};
private:
    dispers_dialog(const dispers_dialog&);
    dispers_dialog &operator=(const dispers_dialog&);

public:
    dispers_dialog(FXWindow *w, struct stack *s);

    long on_cmd_layer_change(FXObject*,FXSelector,void*);
    long on_cmd_plot(FXObject*,FXSelector,void*);

    enum {
        ID_LAYER_CHANGE = FXDialogBox::ID_LAST,
        ID_PLOT,
        ID_LAST
    };
};

#endif
