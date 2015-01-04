#include "recipe_window.h"

// Map
FXDEFMAP(recipe_window) recipe_window_map[]= {
#if 0
    FXMAPFUNCS(SEL_LEFTBUTTONPRESS, recipe_window::ID_FILM_MENU, recipe_window::ID_FILM_MENU_LAST, recipe_window::on_cmd_film_menu),
    FXMAPFUNCS(SEL_CHANGED, recipe_window::ID_FILM_NAME, recipe_window::ID_FILM_NAME_LAST, recipe_window::on_change_name),
    FXMAPFUNCS(SEL_CHANGED, recipe_window::ID_FILM_THICKNESS, recipe_window::ID_FILM_THICKNESS_LAST, recipe_window::on_change_thickness),
    FXMAPFUNC(SEL_COMMAND, recipe_window::ID_INSERT_LAYER, recipe_window::on_cmd_insert_layer),
    FXMAPFUNC(SEL_COMMAND, recipe_window::ID_DELETE_LAYER, recipe_window::on_cmd_delete_layer),
    FXMAPFUNC(SEL_COMMAND, recipe_window::ID_REPLACE_LAYER, recipe_window::on_cmd_replace_layer),
#endif
};

FXIMPLEMENT(recipe_window,FXDialogBox,recipe_window_map,ARRAYNUMBER(recipe_window_map));

recipe_window::recipe_window(fit_recipe *rcp, FXApp* a, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(a, "Recipe Edit", opts, 0, 0, 540, 420, pl, pr, pt, pb, hs, vs),
    recipe(rcp)
{
    FXVerticalFrame *vf = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXSpring *topspr = new FXSpring(vf, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 70);
    FXHorizontalFrame *tophf = new FXHorizontalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXHorizontalSeparator(vf,SEPARATOR_GROOVE|LAYOUT_FILL_X);

    FXSpring *botspr = new FXSpring(vf, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 30);
}

recipe_window::~recipe_window()
{
}
