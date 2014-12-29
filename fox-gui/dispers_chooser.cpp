#include "dispers_chooser.h"

#include "disp_library_iter.h"
#include "dispers_ui_edit.h"

class library_dispers_selector : public dispers_selector {
public:
    library_dispers_selector(FXComboBox *c): combo(c) {}
    virtual disp_t *get();
    FXComboBox *combo;
};

disp_t *library_dispers_selector::get()
{
    FXString name = this->combo->getText();
    disp_t *d = dispers_library_search(name.text());
    return d;
}

// Map
FXDEFMAP(dispers_chooser) dispers_chooser_map[]= {
    FXMAPFUNC(SEL_COMMAND, dispers_chooser::ID_CATEGORY, dispers_chooser::on_cmd_category),
    FXMAPFUNC(SEL_COMMAND, dispers_chooser::ID_DISPERS, dispers_chooser::on_cmd_dispers),
};

FXIMPLEMENT(dispers_chooser,FXDialogBox,dispers_chooser_map,ARRAYNUMBER(dispers_chooser_map));

FXWindow *
new_library_chooser(dispers_chooser *chooser, dispers_selector **pselect, FXComposite *win)
{
    disp_library_iter disp_iter;

    FXHorizontalFrame *hf = new FXHorizontalFrame(win, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(hf, "Library Model");

    int nb_disp = disp_iter.length();

    FXComboBox *combo = new FXComboBox(hf, 10, chooser, dispers_chooser::ID_DISPERS, COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK);
    combo->setNumVisible(nb_disp);
    for(const char *nm = disp_iter.start(); nm; nm = disp_iter.next()) {
        combo->appendItem(nm);
    }

    *pselect = new library_dispers_selector(combo);

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

    vframe = new FXVerticalFrame(hf,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    choose_switcher = new FXSwitcher(vframe, LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|FRAME_THICK|FRAME_RAISED, 0, 0, 500, 80);
    new_library_chooser(this, this->dispers_selectors + 0, choose_switcher);
    new FXHorizontalFrame(choose_switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    this->dispers_selectors[1] = NULL;
    new FXHorizontalFrame(choose_switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    this->dispers_selectors[2] = NULL;
    new FXHorizontalFrame(choose_switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    this->dispers_selectors[3] = NULL;

    dispwin = new FXLabel(vframe, "Choose a dispersion");
}

dispers_chooser::~dispers_chooser()
{
    for (int i = 0; i < 4; i++) {
        delete dispers_selectors[i];
    }
}

long
dispers_chooser::on_cmd_category(FXObject *, FXSelector, void *)
{
    int cat = catlist->getCurrentItem();
    choose_switcher->setCurrent(cat);
    fprintf(stderr, ">> command %d\n", cat);
    return 1;
}

long
dispers_chooser::on_cmd_dispers(FXObject *, FXSelector, void *)
{
    int cat = catlist->getCurrentItem();
    dispers_selector *dispers_select = dispers_selectors[cat];
    if (dispers_select) {
        disp_t *d = dispers_select->get();
        if (d->type == DISP_HO) {
            delete dispwin;
            dispwin = new fx_disp_ho_window(d, vframe, LAYOUT_FILL_X|LAYOUT_FILL_Y);
            dispwin->create();
            vframe->recalc();
        }
        fprintf(stderr, ">> dispersion: %p\n", d);
    }
    return 1;
}
