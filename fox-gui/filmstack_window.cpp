#include "filmstack_window.h"
#include "dispers_chooser.h"

// Map
FXDEFMAP(filmstack_window) filmstack_window_map[]= {
    FXMAPFUNC(SEL_RIGHTBUTTONRELEASE, filmstack_window::ID_STACK, filmstack_window::on_stack_right_mouse),
    FXMAPFUNC(SEL_COMMAND, filmstack_window::ID_INSERT_LAYER, filmstack_window::on_cmd_insert_layer),
    FXMAPFUNC(SEL_COMMAND, filmstack_window::ID_DELETE_LAYER, filmstack_window::on_cmd_delete_layer),
};

FXIMPLEMENT(filmstack_window,FXDialogBox,filmstack_window_map,ARRAYNUMBER(filmstack_window_map));

filmstack_window::filmstack_window(FXApp* a, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(a, "Film Stack", opts, 0, 0, 200, 300, pl, pr, pt, pb, hs, vs)
{
    layersframe = new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXButton *lab1 = new FXButton(layersframe, "Silicon", NULL, this, ID_STACK, FRAME_LINE|JUSTIFY_NORMAL|LAYOUT_FILL_X|LAYOUT_BOTTOM);
    FXButton *lab2 = new FXButton(layersframe, "void", NULL, this, ID_STACK, FRAME_LINE|JUSTIFY_NORMAL|LAYOUT_FILL_X|LAYOUT_BOTTOM);

    popupmenu = new FXMenuPane(this);
    new FXMenuCommand(popupmenu,"Delete Layer", NULL, this, ID_DELETE_LAYER);
    new FXMenuCommand(popupmenu,"Insert Layer Above", NULL, this, ID_INSERT_LAYER);
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

static int get_layer_number(FXWindow *frame, FXObject *child) {
    int n = 0;
    for (FXWindow* w = frame->getFirst(); w != 0; w = w->getNext(), n++) {
        if ((FXObject*)w == child) {
            return n;
        }
    }
    return -1;
}

static FXWindow *get_child(FXWindow *frame, int n) {
    int i = 0;
    for (FXWindow* w = frame->getFirst(); w != 0; w = w->getNext(), i++) {
        if (i == n) {
            return w;
        }
    }
    return 0;
}

long
filmstack_window::on_stack_right_mouse(FXObject*sender,FXSelector,void* ptr) {
    FXEvent* event=(FXEvent*)ptr;
    current_layer = get_layer_number(layersframe, sender);
    if(!event->moved){
        popupmenu->popup(NULL,event->root_x,event->root_y);
        getApp()->runModalWhileShown(popupmenu);
    }
    return 1;
}

long
filmstack_window::on_cmd_insert_layer(FXObject*,FXSelector,void*)
{
    static int counter = 0;

    dispers_chooser *chooser = new dispers_chooser(this->getApp());
    chooser->execute();

    FXWindow *ref = get_child(layersframe, current_layer + 1);
    if (!ref) return 0;
    char label[] = "New Layer X";
    label[strlen(label) - 1] = 'A' + counter++;
    FXButton *button = new FXButton(layersframe, label, NULL, this, ID_STACK, FRAME_LINE|JUSTIFY_NORMAL|LAYOUT_FILL_X|LAYOUT_BOTTOM);
    button->create();
    button->reparent(layersframe, ref);
    return 1;
}

long
filmstack_window::on_cmd_delete_layer(FXObject*,FXSelector,void*)
{
    FXWindow *bot = get_child(layersframe, current_layer - 1);
    FXWindow *cur = get_child(layersframe, current_layer);
    FXWindow *top = get_child(layersframe, current_layer + 1);
    if (bot) {
        bot->reparent(layersframe, top);
        delete cur;
    }
    return 1;
}
