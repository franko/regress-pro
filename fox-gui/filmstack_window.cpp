#include "filmstack_window.h"

// Map
FXDEFMAP(filmstack_window) filmstack_window_map[]= {
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, filmstack_window::ID_STACK, filmstack_window::on_stack_right_mouse),
};

FXIMPLEMENT(filmstack_window,FXDialogBox,filmstack_window_map,ARRAYNUMBER(filmstack_window_map));

filmstack_window::filmstack_window(FXApp* a, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(a, "Film Stack", opts, 0, 0, 200, 300, pl, pr, pt, pb, hs, vs)
{
    FXVerticalFrame *vfr = new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXLabel *lab1 = new FXButton(vfr, "Silicon", NULL, this, ID_STACK, FRAME_LINE|JUSTIFY_NORMAL|LAYOUT_FILL_X);
    FXLabel *lab2 = new FXButton(vfr, "void", NULL, this, ID_STACK, FRAME_LINE|JUSTIFY_NORMAL|LAYOUT_FILL_X);

    popupmenu = new FXMenuPane(this);
    new FXMenuCommand(popupmenu,"Delete Layer",NULL,this,filmstack_window::ID_DELETE_LAYER);
    new FXMenuCommand(popupmenu,"Insert Layer Above",NULL,this,filmstack_window::ID_INSERT_LAYER);
}

filmstack_window::~filmstack_window()
{
    delete popupmenu;
}

void filmstack_window::create()
{
    FXDialogBox::create();
    popupmenu->create();
}

long
filmstack_window::on_stack_right_mouse(FXObject*,FXSelector,void* ptr) {
    fprintf(stderr, ">> right mouse event\n");
    FXEvent* event=(FXEvent*)ptr;
    if(!event->moved){
        popupmenu->popup(NULL,event->root_x,event->root_y);
        getApp()->runModalWhileShown(popupmenu);
    }
    return 1;
}
