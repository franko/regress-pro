#include "dispers_chooser.h"
#include "regress_pro.h"
#include "disp_library_iter.h"
#include "dispers_ui_edit.h"
#include "glass_sellmeier_data.h"
#include "error-messages.h"
#include "str-util.h"
#include "lexer.h"

FXIMPLEMENT(fx_dispers_selector,FXHorizontalFrame,NULL,0);

class fx_library_selector : public fx_dispers_selector {
public:
    fx_library_selector(FXWindow *chooser, const char *name, disp_list *list, FXComposite *p, FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
    virtual disp_t *get_dispersion();
    virtual void reset();
private:
    disp_library_iter iter;
    FXComboBox *combo;
};

fx_library_selector::fx_library_selector(FXWindow *chooser, const char *name, disp_list *list, FXComposite *p, FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
    fx_dispers_selector(chooser, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs),
    iter(list)
{
    new FXLabel(this, name);
    int nb_disp = iter.length();

    combo = new FXComboBox(this, 10, chooser, dispers_chooser::ID_DISPERS, COMBOBOX_STATIC|FRAME_SUNKEN);
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
    combo = new FXComboBox(this, 10, chooser, dispers_chooser::ID_DISPERS, COMBOBOX_STATIC|FRAME_SUNKEN);
    combo->setNumVisible(8);
    combo->appendItem("- choose a model");
    combo->appendItem("Harmonic Oscillator");
    combo->appendItem("Cauchy");
    combo->appendItem("Lookup");
    combo->appendItem("Forouhi-Bloomer");
    combo->appendItem("Tauc-Lorentz");
    combo->appendItem("Bruggeman");
    combo->appendItem("Sellmeier");
}

disp_t *
fx_newmodel_selector::get_dispersion()
{
    FXString name = this->combo->getText();
    if (name == "Harmonic Oscillator") {
        struct ho_params param0 = {0.0, 15.7, 0.0, 1.0 / 3.0, 0.0};
        return disp_new_ho("*HO", 1, &param0);
    } else if (name == "Cauchy") {
        double n[3] = { 1.0, 0.0, 0.0 };
        double k[3] = { 0.0, 0.0, 0.0 };
        return disp_new_cauchy("*cauchy", n, k);
    } else if (name == "Lookup") {
        disp_t *comp = disp_list_search(app_lib, "sio2");
        if (comp) {
            return disp_lookup_new_from_comp("*lookup", comp);
        }
    } else if (name == "Forouhi-Bloomer") {
        struct fb_osc osc0 = {2.0, 5.8, 10};
        return disp_new_fb("*FB", FOROUHI_BLOOMER_STANDARD, 1, 1.0, 1.5, &osc0);
    } else if (name == "Tauc-Lorentz") {
        struct fb_osc osc0 = {190, 3.5, 2};
        return disp_new_tauc_lorentz("* TL", TAUC_LORENTZ_STANDARD, 1, 1.0, 1.5, &osc0);
    } else if (name == "Bruggeman") {
        disp_t *comp1 = lib_disp_table_get("sio2");
        struct bema_component components[1];
        components[0].fraction = 0.5;
        components[0].disp = lib_disp_table_get("nitride-2");
        return bruggeman_new("* bruggeman", comp1, 1, components);
    } else if (name == "Sellmeier") {
        // Coefficients for BK7 Crown Glass
        double a[3] = { 1.03961212, 0.231792344, 1.01046945 };
        double b[3] = { 6.00069867e-3, 2.00179144e-2, 1.03560653e+2 };
        return disp_new_sellmeier("*sellmeier", a, b);
    }
    return NULL;
}

void
fx_newmodel_selector::reset()
{
    combo->setCurrentItem(0);
}

static disp_list files_list[1] = {{NULL, NULL}};

static const FXchar disp_file_patterns[] =
    "Dispersion files (*.mat,*.nk,*.dsp)"
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
    virtual disp_t *get_dispersion();
    virtual void reset();

    long on_cmd_choose_file(FXObject *, FXSelector, void *);

    enum {
        ID_CHOOSE_FILE = fx_dispers_selector::ID_LAST,
        ID_LAST
    };
private:
    FXWindow *m_chooser;
    FXComboBox *m_combo;
};

FXDEFMAP(fx_file_disp_selector) fx_file_disp_selector_map[]= {
    FXMAPFUNC(SEL_COMMAND, fx_file_disp_selector::ID_CHOOSE_FILE, fx_file_disp_selector::on_cmd_choose_file),
};

FXIMPLEMENT(fx_file_disp_selector,fx_dispers_selector,fx_file_disp_selector_map,ARRAYNUMBER(fx_file_disp_selector_map));

fx_file_disp_selector::fx_file_disp_selector(FXWindow *chooser, FXComposite *p, FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs)
    : fx_dispers_selector(chooser, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs),
    m_chooser(chooser)
{
    m_combo = new FXComboBox(this, 10, chooser, dispers_chooser::ID_DISPERS, LAYOUT_FIX_WIDTH|COMBOBOX_STATIC|FRAME_SUNKEN, 0, 0, 160, 0);
    int nb_loaded_disp = disp_list_length(files_list); // loaded_disp_list.number();
    m_combo->setNumVisible(nb_loaded_disp + 1);
    m_combo->appendItem("-- select a file");
    for (disp_node *nd = files_list->first; nd; nd = nd->next) {
        m_combo->appendItem(disp_get_name(nd->content));
    }
    if (nb_loaded_disp == 0) {
        m_combo->disable();
    }
    new FXButton(this, "Browse", NULL, this, ID_CHOOSE_FILE);
}

long fx_file_disp_selector::on_cmd_choose_file(FXObject *, FXSelector, void *)
{
    FXFileDialog open(this,"Open a Dispersion File");
    open.setDirectory(regress_pro_app()->disp_dir);
    open.setPatternList(disp_file_patterns);

    if(open.execute()) {
        FXString filename = open.getFilename();
        regress_pro_app()->disp_dir = open.getDirectory();
        str_ptr error_message;
        disp_t *disp = NULL;
        FXString extension = filename.after('.');
        if (comparecase(extension, "mat") == 0) {
            disp = load_mat_dispers(filename.text(), &error_message);
        } else if (comparecase(extension, "nk") == 0) {
            disp = load_nk_table(filename.text(), &error_message);
        } else if (comparecase(extension, "dsp") == 0) {
            str_t disp_text;
            str_init(disp_text, 128);
            if(str_loadfile(filename.text(), disp_text) != 0) {
                FXMessageBox::error(this, MBOX_OK, "Dispersion Open", "Cannot read file \"%s\".", filename.text());
                str_free(disp_text);
                return 1;
            }

            lexer_t *l = lexer_new(CSTR(disp_text));
            disp = disp_read(l);
            lexer_free(l);
            str_free(disp_text);
            if (!disp) {
                error_message = new_error_message(LOADING_FILE_ERROR, "Cannot open dispersion file \"%s\"", filename.text());
            }
        } else {
            FXMessageBox::error(this, MBOX_OK, "Dispersion Open", "Unknown extension.");
            return 1;
        }
        if (!disp) {
            FXMessageBox::error(this, MBOX_OK, "Dispersion Open", "%s.", CSTR(error_message));
            free_error_message(error_message);
            return 1;
        }
        disp_list_add(files_list, disp, NULL);
        m_combo->appendItem(disp_get_name(disp));
        int items_nb = m_combo->getNumItems();
        m_combo->setCurrentItem(items_nb - 1);
        if (items_nb > 1) {
            m_combo->enable();
        }
        if (items_nb <= 10) {
            m_combo->setNumVisible(items_nb);
        }
        m_chooser->handle(this, FXSEL(SEL_COMMAND, dispers_chooser::ID_DISPERS), NULL);
    }
    return 1;
}

disp_t *
fx_file_disp_selector::get_dispersion()
{
    FXint index = m_combo->getCurrentItem() - 1;
    if (index < 0) return NULL;
    return disp_list_get_by_index(files_list, index);
}

void
fx_file_disp_selector::reset()
{
    m_combo->setCurrentItem(0);
}

class fx_glass_selector : public fx_dispers_selector {
public:
    fx_glass_selector(FXWindow *chooser, const char *name, FXComposite *p, FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
    virtual disp_t *get_dispersion();
    virtual void reset();
private:
    FXComboBox *combo;
};

fx_glass_selector::fx_glass_selector(FXWindow *chooser, const char *name, FXComposite *p, FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
    fx_dispers_selector(chooser, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{
    new FXLabel(this, name);
    combo = new FXComboBox(this, 10, chooser, dispers_chooser::ID_DISPERS, COMBOBOX_STATIC|FRAME_SUNKEN);
    combo->setNumVisible(GLASS_DATA_SIZE + 1);
    combo->appendItem("- choose a dispersion");
    for (int i = 0; i < GLASS_DATA_SIZE; i++) {
        combo->appendItem(glass_coeff_table[i].name);
    }
}

disp_t *
fx_glass_selector::get_dispersion()
{
    FXint index = combo->getCurrentItem() - 1;
    const struct glass_coeff *glass = &glass_coeff_table[index];
    disp_t *new_disp = disp_new_sellmeier(glass->name, glass->coeff, &glass->coeff[3]);
    disp_set_info_wavelength(new_disp, 326, 1129);
    return new_disp;
}

void
fx_glass_selector::reset()
{
    combo->setCurrentItem(0);
}

// Map
FXDEFMAP(dispers_chooser) dispers_chooser_map[]= {
    FXMAPFUNC(SEL_COMMAND, dispers_chooser::ID_CATEGORY, dispers_chooser::on_cmd_category),
    FXMAPFUNC(SEL_COMMAND, dispers_chooser::ID_DISPERS, dispers_chooser::on_cmd_dispers),
};

FXIMPLEMENT(dispers_chooser,FXDialogBox,dispers_chooser_map,ARRAYNUMBER(dispers_chooser_map));

dispers_chooser::dispers_chooser(FXWindow* win, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(win, "Dispersion Select", opts, 0, 0, 575, 420, pl, pr, pt, pb, hs, vs),
    current_disp(0)
{
    FXHorizontalFrame *hf = new FXHorizontalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXSpring *listspring = new FXSpring(hf, LAYOUT_FILL_X|LAYOUT_FILL_Y, 20, 0);
    catlist = new FXList(listspring, this, ID_CATEGORY, LIST_SINGLESELECT|LAYOUT_FILL_Y|LAYOUT_FILL_X);
    catlist->appendItem("Library", NULL, NULL, TRUE);
    catlist->appendItem("Choose File", NULL, NULL, TRUE);
    catlist->appendItem("New Model", NULL, NULL, TRUE);
    catlist->appendItem("User List", NULL, NULL, TRUE);
    catlist->appendItem("Preset List", NULL, NULL, TRUE);
    catlist->appendItem("Glass Library", NULL, NULL, TRUE);
    catlist->selectItem(0, FALSE);

    FXSpring *vframespring = new FXSpring(hf, LAYOUT_FILL_X|LAYOUT_FILL_Y, 80, 0);
    vframe = new FXVerticalFrame(vframespring,LAYOUT_FILL_X|LAYOUT_FILL_Y);
    choose_switcher = new FXSwitcher(vframe, LAYOUT_FILL_X|FRAME_GROOVE);
    new fx_library_selector(this, "Library Model",  app_lib, choose_switcher);
    new fx_file_disp_selector(this, choose_switcher);
    new fx_newmodel_selector(this, choose_switcher);
    new fx_library_selector(this, "User Model", user_lib, choose_switcher);
    new fx_library_selector(this, "Preset Model", preset_lib, choose_switcher);
    new fx_glass_selector(this, "Glass Library", choose_switcher);

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
    FXHorizontalFrame *labfr = new FXHorizontalFrame(frame, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_NONE);
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
