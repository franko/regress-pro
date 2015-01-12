#ifndef DISPERS_EDIT_WINDOW_H
#define DISPERS_EDIT_WINDOW_H

#include <fx.h>

#include "dispers_ui_edit.h"
#include "dispers.h"

class dispers_edit_window : public FXDialogBox {
    FXDECLARE(dispers_edit_window)

protected:
    dispers_edit_window() {};
private:
    dispers_edit_window(const dispers_edit_window&);
    dispers_edit_window &operator=(const dispers_edit_window&);

public:
    dispers_edit_window(disp_t *d, FXWindow *win, FXuint opts=DECOR_TITLE|DECOR_BORDER,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=10,FXint pr=10,FXint pt=10,FXint pb=10,FXint hs=4,FXint vs=4);

private:
	fx_disp_window *disp_window;
};

#endif
