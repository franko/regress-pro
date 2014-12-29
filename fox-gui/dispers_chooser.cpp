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

class new_dispers_selector : public dispers_selector {
public:
    new_dispers_selector(FXComboBox *c): combo(c) {}
    virtual disp_t *get();
    FXComboBox *combo;
};

disp_t *new_dispers_selector::get()
{
    FXString name = this->combo->getText();
    if (name == "Harmonic Oscillator") {
        struct ho_params param0 = {0.0, 15.7, 0.0, 1.0 / 3.0, 0.0};
        return disp_new_ho("*HO", 1, &param0);
    } else if (name == "Cauchy Model") {
        double n[3] = { 1.0, 0.0, 0.0 };
        double k[3] = { 1.0, 0.0, 0.0 };
        return disp_new_cauchy("*cauchy", n, k);
    }
    return NULL;
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

FXWindow *
new_model_chooser(dispers_chooser *chooser, dispers_selector **pselect, FXComposite *win)
{
    FXHorizontalFrame *hf = new FXHorizontalFrame(win, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(hf, "New Model");

    FXComboBox *combo = new FXComboBox(hf, 10, chooser, dispers_chooser::ID_DISPERS, COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK);
    combo->setNumVisible(2);
    combo->appendItem("Harmonic Oscillator");
    combo->appendItem("Cauchy Model");

    *pselect = new new_dispers_selector(combo);

    return hf;
}

dispers_chooser::dispers_chooser(FXApp* a, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(a, "Film Stack", opts, 0, 0, 500, 400, pl, pr, pt, pb, hs, vs),
    current_disp(0)
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
    new_model_chooser(this, this->dispers_selectors + 2, choose_switcher);
    new FXHorizontalFrame(choose_switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    this->dispers_selectors[3] = NULL;

    dispwin = new FXLabel(vframe, "Choose a dispersion");

    validhf = new FXHorizontalFrame(vframe, LAYOUT_FILL_X);
    new FXButton(validhf, "Cancel", NULL, this, ID_CANCEL);
    new FXButton(validhf, "Ok", NULL, this, ID_ACCEPT);
}

disp_t *dispers_chooser::get_dispersion()
{
    disp_t *d = current_disp;
    current_disp = 0;
    return d;
}

dispers_chooser::~dispers_chooser()
{
    for (int i = 0; i < 4; i++) {
        delete dispers_selectors[i];
    }
    if (current_disp) {
        disp_free(current_disp);
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
        if (current_disp) {
            disp_free(current_disp);
        }
        current_disp = dispers_select->get();
        if (current_disp->type == DISP_HO) {
            delete dispwin;
            dispwin = new fx_disp_ho_window(current_disp, vframe, LAYOUT_FILL_X|LAYOUT_FILL_Y);
            dispwin->create();
            dispwin->reparent(vframe, validhf);
        }
    }
    return 1;
}
