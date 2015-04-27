#include "dispers_chooser.h"

#include "disp_library_iter.h"
#include "dispers_ui_edit.h"
#include "error-messages.h"

FXIMPLEMENT(fx_dispers_selector,FXHorizontalFrame,NULL,0);

class fx_library_selector : public fx_dispers_selector {
public:
    fx_library_selector(FXWindow *chooser, FXComposite *p, FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
    virtual disp_t *get_dispersion();
    virtual void reset();
private:
    disp_library_iter iter;
    FXComboBox *combo;
};

fx_library_selector::fx_library_selector(FXWindow *chooser, FXComposite *p, FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs)
: fx_dispers_selector(chooser, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{
    new FXLabel(this, "Library Model");
    int nb_disp = iter.length();

    combo = new FXComboBox(this, 10, chooser, dispers_chooser::ID_DISPERS, COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK);
    combo->setNumVisible(nb_disp + 1);
    combo->appendItem("- choose a dispersion");
    for(const char *nm = iter.start(); nm; nm = iter.next()) {
        combo->appendItem(nm);
    }
}

disp_t *
fx_library_selector::get_dispersion()
{
    FXint index = combo->getCurrentItem() - 1;
    return iter.get(index);
}

void
fx_library_selector::reset()
{
    combo->setCurrentItem(0);
}


class fx_userlib_selector : public fx_dispers_selector {
public:
    fx_userlib_selector(FXWindow *chooser, FXComposite *p, FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
    virtual disp_t *get_dispersion();
    virtual void reset();
private:
    userlib_iter iter;
    FXComboBox *combo;
};

fx_userlib_selector::fx_userlib_selector(FXWindow *chooser, FXComposite *p, FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs)
: fx_dispers_selector(chooser, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{
    new FXLabel(this, "User Model");
    int nb_disp = iter.length();

    combo = new FXComboBox(this, 10, chooser, dispers_chooser::ID_DISPERS, COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK);
    combo->setNumVisible(nb_disp + 1);
    combo->appendItem("- choose a dispersion");
    for(const char *nm = iter.start(); nm; nm = iter.next()) {
        combo->appendItem(nm);
    }
}

disp_t *
fx_userlib_selector::get_dispersion()
{
    FXint index = combo->getCurrentItem() - 1;
    return iter.get(index);
}

void
fx_userlib_selector::reset()
{
    combo->setCurrentItem(0);
}


class fx_newmodel_selector : public fx_dispers_selector {
public:
    fx_newmodel_selector(FXWindow *chooser, FXComposite *p, FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
    virtual disp_t *get_dispersion();
    virtual void reset();
private:
    FXComboBox *combo;
};


fx_newmodel_selector::fx_newmodel_selector(FXWindow *chooser, FXComposite *p, FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs)
: fx_dispers_selector(chooser, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{
    new FXLabel(this, "New Model");
    combo = new FXComboBox(this, 10, chooser, dispers_chooser::ID_DISPERS, COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK);
    combo->setNumVisible(3);
    combo->appendItem("- choose a model");
    combo->appendItem("Harmonic Oscillator");
    combo->appendItem("Cauchy Model");
}

disp_t *
fx_newmodel_selector::get_dispersion()
{
    FXString name = this->combo->getText();
    if (name == "Harmonic Oscillator") {
        struct ho_params param0 = {0.0, 15.7, 0.0, 1.0 / 3.0, 0.0};
        return disp_new_ho("*HO", 1, &param0);
    } else if (name == "Cauchy Model") {
        double n[3] = { 1.0, 0.0, 0.0 };
        double k[3] = { 0.0, 0.0, 0.0 };
        return disp_new_cauchy("*cauchy", n, k);
    }
    return NULL;
}

void
fx_newmodel_selector::reset()
{
    combo->setCurrentItem(0);
}

static const FXchar disp_file_patterns[] =
    "MAT files (*.mat)"
    "\nNK tables (*.nk)"
    "\nDispersion files (*.dsp)"
    "\nAll Files (*)";

class fx_file_disp_selector : public fx_dispers_selector {
    FXDECLARE(fx_file_disp_selector)
protected:
    fx_file_disp_selector() {};
private:
    fx_file_disp_selector(const fx_file_disp_selector&);
    fx_file_disp_selector &operator=(const fx_file_disp_selector&);
public:
    fx_file_disp_selector(FXWindow *chooser, FXComposite *p, FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
    ~fx_file_disp_selector();
    virtual disp_t *get_dispersion();
    virtual void reset();

    long on_cmd_choose_file(FXObject *, FXSelector, void *);

    enum {
        ID_CHOOSE_FILE = fx_dispers_selector::ID_LAST,
        ID_LAST
    };
private:
    void set_dispersion(disp_t *d);

    disp_t *m_disp;
    FXWindow *m_chooser;
};

FXDEFMAP(fx_file_disp_selector) fx_file_disp_selector_map[]= {
    FXMAPFUNC(SEL_COMMAND, fx_file_disp_selector::ID_CHOOSE_FILE, fx_file_disp_selector::on_cmd_choose_file),
};

FXIMPLEMENT(fx_file_disp_selector,fx_dispers_selector,fx_file_disp_selector_map,ARRAYNUMBER(fx_file_disp_selector_map));

fx_file_disp_selector::fx_file_disp_selector(FXWindow *chooser, FXComposite *p, FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs)
    : fx_dispers_selector(chooser, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs),
    m_disp(NULL), m_chooser(chooser)
{
    new FXButton(this, "Choose", NULL, this, ID_CHOOSE_FILE);
}

fx_file_disp_selector::~fx_file_disp_selector()
{
    if (m_disp) {
        disp_free(m_disp);
    }
}

void fx_file_disp_selector::set_dispersion(disp_t *d)
{
    if (m_disp) {
        disp_free(m_disp);
    }
    m_disp = d;
}

long fx_file_disp_selector::on_cmd_choose_file(FXObject *, FXSelector, void *)
{
    FXFileDialog open(this,"Open a Dispersion File");
    open.setPatternList(disp_file_patterns);

    if(open.execute()) {
        FXString filename = open.getFilename();
        str_ptr error_message;
        disp_t *disp = NULL;
        if (filename.right(4) == ".mat") {
            disp = load_mat_dispers(filename.text(), &error_message);
        } else if (filename.right(3) == ".nk") {
            disp = load_nk_table(filename.text(), &error_message);
        } else {
            FXMessageBox::error(this, MBOX_OK, "Dispersion Open", "Unknown extension.");
            return 1;
        }
        if (!disp) {
            FXMessageBox::error(this, MBOX_OK, "Dispersion Open", "%s.", CSTR(error_message));
            free_error_message(error_message);
            return 1;
        }
        m_disp = disp;
        m_chooser->handle(this, FXSEL(SEL_COMMAND, dispers_chooser::ID_DISPERS), NULL);
    }
    return 1;
}

disp_t *
fx_file_disp_selector::get_dispersion()
{
    disp_t *d = m_disp;
    m_disp = NULL;
    return d;
}

void
fx_file_disp_selector::reset()
{
    set_dispersion(NULL);
}

// Map
FXDEFMAP(dispers_chooser) dispers_chooser_map[]= {
    FXMAPFUNC(SEL_COMMAND, dispers_chooser::ID_CATEGORY, dispers_chooser::on_cmd_category),
    FXMAPFUNC(SEL_COMMAND, dispers_chooser::ID_DISPERS, dispers_chooser::on_cmd_dispers),
};

FXIMPLEMENT(dispers_chooser,FXDialogBox,dispers_chooser_map,ARRAYNUMBER(dispers_chooser_map));

dispers_chooser::dispers_chooser(FXWindow* win, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(win, "Dispersion Select", opts, 0, 0, 480, 340, pl, pr, pt, pb, hs, vs),
    current_disp(0)
{
    FXHorizontalFrame *hf = new FXHorizontalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXSpring *listspring = new FXSpring(hf, LAYOUT_FILL_X|LAYOUT_FILL_Y, 20, 0);
    catlist = new FXList(listspring, this, ID_CATEGORY, LIST_SINGLESELECT|LAYOUT_FILL_Y|LAYOUT_FILL_X);
    catlist->appendItem("Library", NULL, NULL, TRUE);
    catlist->appendItem("Choose File", NULL, NULL, TRUE);
    catlist->appendItem("New Model", NULL, NULL, TRUE);
    catlist->appendItem("User List", NULL, NULL, TRUE);

    FXSpring *vframespring = new FXSpring(hf, LAYOUT_FILL_X|LAYOUT_FILL_Y, 80, 0);
    vframe = new FXVerticalFrame(vframespring,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    choose_switcher = new FXSwitcher(vframe, LAYOUT_FILL_X|FRAME_GROOVE);
    new fx_library_selector(this, choose_switcher);
    new fx_file_disp_selector(this, choose_switcher);
    new fx_newmodel_selector(this, choose_switcher);
    new fx_userlib_selector(this, choose_switcher);

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
    selector_frame(cat)->reset();
    release_current_disp();
    clear_dispwin();
    return 1;
}

long
dispers_chooser::on_cmd_dispers(FXObject *, FXSelector, void *)
{
    int cat = choose_switcher->getCurrent();
    fx_dispers_selector *sel = selector_frame(cat);
    disp_t *new_disp = (sel ? sel->get_dispersion() : NULL);
    if (new_disp) {
        release_current_disp();
        current_disp = new_disp;
        FXWindow *new_dispwin = new_disp_window(current_disp, vframe);
        replace_dispwin(new_dispwin);
    }
    return 1;
}
