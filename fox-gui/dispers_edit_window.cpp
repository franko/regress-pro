#include "dispers_edit_window.h"

// Map
FXDEFMAP(dispers_edit_window) dispers_edit_window_map[]= {
};

FXIMPLEMENT(dispers_edit_window,FXDialogBox,dispers_edit_window_map,ARRAYNUMBER(dispers_edit_window_map));

dispers_edit_window::dispers_edit_window(disp_t *d, FXWindow *topwin, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(topwin, "Dispersion Edit", opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{
	FXVerticalFrame *frame = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
	disp_window = new_disp_window(d, frame);

    new FXHorizontalSeparator(frame, SEPARATOR_GROOVE|LAYOUT_FILL_X);
    FXHorizontalFrame *validhf = new FXHorizontalFrame(frame, LAYOUT_FILL_X);
    new FXButton(validhf,"&Cancel",NULL,this,ID_CANCEL,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);
    new FXButton(validhf,"&Ok",NULL,this,ID_ACCEPT,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);
}
