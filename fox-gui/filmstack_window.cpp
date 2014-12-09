#include "filmstack_window.h"

// Map
FXDEFMAP(filmstack_window) filmstack_window_map[]= {
//    FXMAPFUNC(SEL_COMMAND, BatchDialog::ID_BROWSE, BatchDialog::onCmdBrowse),
//    FXMAPFUNC(SEL_COMMAND, BatchDialog::ID_ACCEPT, BatchDialog::onCmdRun),
};

FXIMPLEMENT(filmstack_window,FXDialogBox,filmstack_window_map,ARRAYNUMBER(filmstack_window_map));

filmstack_window::filmstack_window(FXApp* a, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(a, "Film Stack", opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{
    FXVerticalFrame *vfr = new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXLabel *lab1 = new FXLabel(vfr, "Silicon", NULL, LAYOUT_FILL_X);
    FXLabel *lab2 = new FXLabel(vfr, "void", NULL, LAYOUT_FILL_X);
}

filmstack_window::~filmstack_window()
{
}
