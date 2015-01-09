#include "filmstack_window.h"
#include "dispers_chooser.h"
#include "recipe_window.h"
#include "regress_pro_window.h"

// Map
FXDEFMAP(filmstack_window) filmstack_window_map[]= {
    FXMAPFUNCS(SEL_LEFTBUTTONPRESS, filmstack_window::ID_FILM_MENU, filmstack_window::ID_FILM_MENU_LAST, filmstack_window::on_cmd_film_menu),
    FXMAPFUNCS(SEL_CHANGED, filmstack_window::ID_FILM_NAME, filmstack_window::ID_FILM_NAME_LAST, filmstack_window::on_change_name),
    FXMAPFUNCS(SEL_CHANGED, filmstack_window::ID_FILM_THICKNESS, filmstack_window::ID_FILM_THICKNESS_LAST, filmstack_window::on_change_thickness),
    FXMAPFUNC(SEL_COMMAND, filmstack_window::ID_INSERT_LAYER, filmstack_window::on_cmd_insert_layer),
    FXMAPFUNC(SEL_COMMAND, filmstack_window::ID_DELETE_LAYER, filmstack_window::on_cmd_delete_layer),
    FXMAPFUNC(SEL_COMMAND, filmstack_window::ID_REPLACE_LAYER, filmstack_window::on_cmd_replace_layer),
    FXMAPFUNC(SEL_COMMAND, filmstack_window::ID_DELETE, recipe_window::onCmdHide),
};

FXIMPLEMENT(filmstack_window,FXDialogBox,filmstack_window_map,ARRAYNUMBER(filmstack_window_map));

filmstack_window::filmstack_window(stack_t *s, FXWindow *topwin, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(topwin, "Film Stack", opts, 0, 0, 340, 200, pl, pr, pt, pb, hs, vs),
    stack(s)
{
    stack_window = setup_stack_window(this);

    popupmenu = new FXMenuPane(this);
    new FXMenuCommand(popupmenu,"Delete Layer", NULL, this, ID_DELETE_LAYER);
    new FXMenuCommand(popupmenu,"Select New Layer", NULL, this, ID_REPLACE_LAYER);
    new FXMenuCommand(popupmenu,"Insert Layer Above", NULL, this, ID_INSERT_LAYER);
}

filmstack_window::~filmstack_window()
{
    delete popupmenu;
}

FXWindow *
filmstack_window::setup_stack_window(FXComposite *cont)
{
    FXVerticalFrame *vf = new FXVerticalFrame(cont, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXMatrix *matrix = new FXMatrix(vf, 3, LAYOUT_FILL_X|LAYOUT_BOTTOM|MATRIX_BY_COLUMNS);
    FXString nstr;
    for (int i = stack->nb - 1; i >= 0; i--) {
        nstr.format("%d", i);
        new FXButton(matrix, nstr, NULL, this, ID_FILM_MENU + i);
        FXTextField *filmtf = new FXTextField(matrix, 24, this, ID_FILM_NAME + i, FRAME_SUNKEN|LAYOUT_FILL_COLUMN);
        filmtf->setText(CSTR(stack->disp[i]->name));
        if (i > 0 && i < stack->nb - 1) {
            FXTextField *thktf = new FXTextField(matrix, 6, this, ID_FILM_THICKNESS + i, FRAME_SUNKEN|TEXTFIELD_REAL);
            nstr.format("%g", stack->thickness[i - 1]);
            thktf->setText(nstr);
        } else {
            new FXVerticalFrame(matrix);
        }
    }
    return vf;
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

void
filmstack_window::rebuild_stack_window()
{
    delete stack_window;
    stack_window = setup_stack_window(this);
    stack_window->create();
    notify_stack_change();
}

void
filmstack_window::notify_stack_change()
{
    getOwner()->handle(this, FXSEL(SEL_COMMAND, regress_pro_window::ID_STACK_CHANGE), NULL);
}

long
filmstack_window::on_cmd_insert_layer(FXObject*,FXSelector,void*)
{
    dispers_chooser *chooser = new dispers_chooser(this->getApp());
    if (chooser->execute() == TRUE) {
        disp_t *d = chooser->get_dispersion();
        if (!d) return 0;
        stack_insert_layer(stack, current_layer + 1, d, 0.0);
        rebuild_stack_window();
        return 1;
    }
    return 0;
}

long
filmstack_window::on_cmd_replace_layer(FXObject*,FXSelector,void*)
{
    dispers_chooser *chooser = new dispers_chooser(this->getApp());
    if (chooser->execute() == TRUE) {
        disp_t *d = chooser->get_dispersion();
        if (!d) return 0;
        disp_free(stack->disp[current_layer]);
        stack->disp[current_layer] = d;
        rebuild_stack_window();
        return 1;
    }
    return 0;
}

long
filmstack_window::on_cmd_delete_layer(FXObject*, FXSelector, void*)
{
    stack_delete_layer(stack, current_layer);
    rebuild_stack_window();
    return 1;
}

long
filmstack_window::on_change_name(FXObject*, FXSelector sel, void *data)
{
    int index = FXSELID(sel) - ID_FILM_NAME;
    str_copy_c(stack->disp[index]->name, (FXchar *)data);
    return 1;
}

long
filmstack_window::on_change_thickness(FXObject*, FXSelector sel, void *data)
{
    int index = FXSELID(sel) - ID_FILM_THICKNESS;
    double x = strtod((FXchar *)data, NULL);
    stack->thickness[index - 1] = x;
    return 1;
}
