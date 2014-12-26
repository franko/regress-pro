#include "dispers_chooser.h"

#include "disp_library_iter.h"

// Map
FXDEFMAP(dispers_chooser) dispers_chooser_map[]= {
    FXMAPFUNC(SEL_COMMAND, dispers_chooser::ID_CATEGORY, dispers_chooser::on_cmd_category),
};

FXIMPLEMENT(dispers_chooser,FXDialogBox,dispers_chooser_map,ARRAYNUMBER(dispers_chooser_map));

FXWindow *
new_library_chooser(FXComposite *win)
{
    disp_library_iter disp_iter;

    FXHorizontalFrame *hf = new FXHorizontalFrame(win, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(hf, "Library Model");

    int nb_disp = disp_iter.length();

    FXComboBox *combo = new FXComboBox(hf, 10, NULL, 0, COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK);
    combo->setNumVisible(nb_disp);
    for(const char *nm = disp_iter.start(); nm; nm = disp_iter.next()) {
        fprintf(stderr, ">> adding %s\n", nm);
        combo->appendItem(nm);
    }

    return hf;
}

dispers_chooser::dispers_chooser(FXApp* a, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(a, "Film Stack", opts, 0, 0, 500, 400, pl, pr, pt, pb, hs, vs)
{
    FXHorizontalFrame *hf = new FXHorizontalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    catlist = new FXList(hf, this, ID_CATEGORY, LIST_SINGLESELECT|LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH, 0, 0, 140, 400);
    catlist->appendItem("Library", NULL, NULL, TRUE);
    catlist->appendItem("Choose File", NULL, NULL, TRUE);
    catlist->appendItem("New Model", NULL, NULL, TRUE);
    catlist->appendItem("User List", NULL, NULL, TRUE);

    FXVerticalFrame *vf = new FXVerticalFrame(hf,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    choose_switcher = new FXSwitcher(vf, LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_RIGHT|FRAME_THICK|FRAME_RAISED);
    new_library_chooser(choose_switcher);
    new FXHorizontalFrame(choose_switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXHorizontalFrame(choose_switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXHorizontalFrame(choose_switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    disp_switcher = new FXSwitcher(vf, LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_RIGHT|FRAME_THICK|FRAME_RAISED);
}

dispers_chooser::~dispers_chooser()
{
}

long
dispers_chooser::on_cmd_category(FXObject *, FXSelector, void *)
{
    int cat = catlist->getCurrentItem();
    choose_switcher->setCurrent(cat);
    fprintf(stderr, ">> command %d\n", cat);
    return 1;
}
