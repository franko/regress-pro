#include "filmstack_window.h"
#include "dispers_chooser.h"
#include "dispers.h"
#include "dispers-library.h"

static stack_t *init_stack();

// Map
FXDEFMAP(filmstack_window) filmstack_window_map[]= {
    FXMAPFUNCS(SEL_LEFTBUTTONPRESS, filmstack_window::ID_FILM_MENU, filmstack_window::ID_FILM_MENU_LAST, filmstack_window::on_cmd_film_menu),
    FXMAPFUNC(SEL_COMMAND, filmstack_window::ID_INSERT_LAYER, filmstack_window::on_cmd_insert_layer),
    FXMAPFUNC(SEL_COMMAND, filmstack_window::ID_DELETE_LAYER, filmstack_window::on_cmd_delete_layer),
};

FXIMPLEMENT(filmstack_window,FXDialogBox,filmstack_window_map,ARRAYNUMBER(filmstack_window_map));

filmstack_window::filmstack_window(FXApp* a, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(a, "Film Stack", opts, 0, 0, 300, 200, pl, pr, pt, pb, hs, vs)
{
    stack = init_stack();
    matrix = setup_stack_window(this);

    popupmenu = new FXMenuPane(this);
    new FXMenuCommand(popupmenu,"Delete Layer", NULL, this, ID_DELETE_LAYER);
    new FXMenuCommand(popupmenu,"Insert Layer Above", NULL, this, ID_INSERT_LAYER);
}

filmstack_window::~filmstack_window()
{
    delete popupmenu;
    stack_free(stack);
    free(stack);
}

FXMatrix *
filmstack_window::setup_stack_window(FXComposite *cont)
{
    FXMatrix *matrix = new FXMatrix(cont, 3, LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_SIDE_BOTTOM|MATRIX_BY_COLUMNS);
    FXString nstr;
    for (int i = stack->nb - 1; i >= 0; i--) {
        nstr.format("%d", i);
        new FXButton(matrix, nstr, NULL, this, ID_FILM_MENU + i);
        FXTextField *filmtf = new FXTextField(matrix, 30, this, ID_FILM_NAME + i, FRAME_SUNKEN);
        FXTextField *thktf = new FXTextField(matrix, 6, this, ID_FILM_THICKNESS + i, FRAME_SUNKEN);
        filmtf->setText(CSTR(stack->disp[i]->name));
        if (i > 0) {
            nstr.format("%g", stack->thickness[i - 1]);
        } else {
            nstr = "";
        }
        thktf->setText(nstr);
    }
    return matrix;
}

void filmstack_window::create()
{
    FXDialogBox::create();
    popupmenu->create();
}

long
filmstack_window::on_cmd_film_menu(FXObject*sender, FXSelector sel, void *ptr)
{
    FXEvent *event = (FXEvent *) ptr;
    current_layer = FXSELID(sel) - ID_FILM_MENU;
    if(!event->moved){
        popupmenu->popup(NULL, event->root_x, event->root_y);
        getApp()->runModalWhileShown(popupmenu);
    }
    return 1;
}

long
filmstack_window::on_cmd_insert_layer(FXObject*,FXSelector,void*)
{
    dispers_chooser *chooser = new dispers_chooser(this->getApp());
    if (chooser->execute() == TRUE) {
        disp_t *d = chooser->get_dispersion();
        if (!d) return 0;
        stack_insert_layer(stack, current_layer, d, 0.0);
        delete matrix;
        matrix = setup_stack_window(this);
        matrix->create();
        return 1;
    }
    return 0;
}

long
filmstack_window::on_cmd_delete_layer(FXObject*,FXSelector,void*)
{
    // To be implemented.
    return 1;
}

stack_t *init_stack()
{
    stack_t *s = (stack_t*) emalloc(sizeof(disp_t));
    stack_init(s);
    disp_t *si = dispers_library_search("si");
    disp_t *sio2 = dispers_library_search("sio2");
    disp_t *vac = dispers_library_search("vacuum");
    stack_add_layer(s, si, 0.0);
    stack_add_layer(s, sio2, 10.0);
    stack_add_layer(s, vac, 0.0);
    return s;
}
