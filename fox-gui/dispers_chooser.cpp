#include "dispers_chooser.h"

#include "disp_library_iter.h"
#include "dispers_ui_edit.h"

class library_dispers_selector : public dispers_selector {
public:
    library_dispers_selector(FXComboBox *c): combo(c) {}
    virtual disp_t *get();
    virtual void reset();
    FXComboBox *combo;
};

disp_t *library_dispers_selector::get()
{
    FXString name = this->combo->getText();
    if (name[0] == '-') return NULL;
    disp_t *d = dispers_library_search(name.text());
    return d;
}

void library_dispers_selector::reset()
{
    combo->setCurrentItem(0);
}

class new_dispers_selector : public dispers_selector {
public:
    new_dispers_selector(FXComboBox *c): combo(c) {}
    virtual disp_t *get();
    virtual void reset();
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

void new_dispers_selector::reset()
{
    combo->setCurrentItem(0);
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
    combo->setNumVisible(nb_disp + 1);
    combo->appendItem("- choose a dispersion");
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
    combo->setNumVisible(3);
    combo->appendItem("- choose a model");
    combo->appendItem("Harmonic Oscillator");
    combo->appendItem("Cauchy Model");

    *pselect = new new_dispers_selector(combo);

    return hf;
}

dispers_chooser::dispers_chooser(FXApp* a, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(a, "Dispersion Select", opts, 0, 0, 480, 340, pl, pr, pt, pb, hs, vs),
    current_disp(0)
{
    FXHorizontalFrame *hf = new FXHorizontalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXSpring *listspring = new FXSpring(hf, LAYOUT_FILL_X|LAYOUT_FILL_Y, 16, 0);
    catlist = new FXList(listspring, this, ID_CATEGORY, LIST_SINGLESELECT|LAYOUT_FILL_Y|LAYOUT_FILL_X);
    catlist->appendItem("Library", NULL, NULL, TRUE);
    catlist->appendItem("Choose File", NULL, NULL, TRUE);
    catlist->appendItem("New Model", NULL, NULL, TRUE);
    catlist->appendItem("User List", NULL, NULL, TRUE);

    FXSpring *vframespring = new FXSpring(hf, LAYOUT_FILL_X|LAYOUT_FILL_Y, 84, 0);
    vframe = new FXVerticalFrame(vframespring,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    choose_switcher = new FXSwitcher(vframe, LAYOUT_FILL_X|FRAME_GROOVE);
    new_library_chooser(this, this->dispers_selectors + 0, choose_switcher);
    new FXHorizontalFrame(choose_switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    this->dispers_selectors[1] = NULL;
    new_model_chooser(this, this->dispers_selectors + 2, choose_switcher);
    new FXHorizontalFrame(choose_switcher, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    this->dispers_selectors[3] = NULL;

    dispwin = new_dispwin_dummy(vframe);
    dispwin_dummy = dispwin;

    FXHorizontalSeparator *hsep = new FXHorizontalSeparator(vframe,SEPARATOR_GROOVE|LAYOUT_FILL_X);
    FXHorizontalFrame *validhf = new FXHorizontalFrame(vframe,LAYOUT_FILL_X);
    new FXButton(validhf,"&Cancel",NULL,this,ID_CANCEL,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);
    new FXButton(validhf,"&Ok",NULL,this,ID_ACCEPT,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);

    dispwin_anchor = hsep;
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

void
dispers_chooser::replace_dispwin(FXWindow *new_dispwin)
{
    delete dispwin;
    dispwin = new_dispwin;
    dispwin->create();
    dispwin->reparent(vframe, dispwin_anchor);
}

FXWindow *
dispers_chooser::new_dispwin_dummy(FXComposite *frame)
{
    FXHorizontalFrame *labfr = new FXHorizontalFrame(frame, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_GROOVE);
    new FXLabel(labfr, "Choose a dispersion", NULL, LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
    dispwin_dummy = labfr;
    return labfr;
}

void
dispers_chooser::clear_dispwin()
{
    if (dispwin == dispwin_dummy) return;
    replace_dispwin(new_dispwin_dummy(vframe));
}

long
dispers_chooser::on_cmd_category(FXObject *, FXSelector, void *)
{
    int cat = catlist->getCurrentItem();
    choose_switcher->setCurrent(cat);
    dispers_selector *dispers_select = dispers_selectors[cat];
    if (dispers_select) {
        dispers_select->reset();
    }
    release_current_disp();
    clear_dispwin();
    return 1;
}

long
dispers_chooser::on_cmd_dispers(FXObject *, FXSelector, void *)
{
    int cat = catlist->getCurrentItem();
    dispers_selector *dispers_select = dispers_selectors[cat];
    if (dispers_select) {
        disp_t *new_disp = dispers_select->get();
        if (!new_disp) return 0;
        release_current_disp();
        current_disp = new_disp;
        FXWindow *new_dispwin = new_disp_window(current_disp, vframe);
        replace_dispwin(new_dispwin);
    }
    return 1;
}
