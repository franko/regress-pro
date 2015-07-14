#include "dispers_ui_edit.h"
#include "disp-ho.h"
#include "disp-fb.h"
#include "dispers_ui_utils.h"
#include "regress_pro.h"

// Map
FXDEFMAP(fx_disp_window) fx_disp_window_map[]= {
    FXMAPFUNC(SEL_CHANGED, fx_disp_window::ID_NAME, fx_disp_window::on_changed_name),
    FXMAPFUNC(SEL_COMMAND, fx_disp_window::ID_DISP_ELEMENT_ADD, fx_disp_window::on_disp_element_add),
    FXMAPFUNCS(SEL_COMMAND, fx_disp_window::ID_DISP_ELEMENT_DELETE, fx_disp_window::ID_DISP_ELEMENT_DELETE_LAST, fx_disp_window::on_disp_element_delete),
    FXMAPFUNCS(SEL_CHANGED, fx_disp_ho_window::ID_PARAM_0, fx_disp_ho_window::ID_PARAM_LAST, fx_disp_ho_window::on_cmd_value),
};

FXIMPLEMENT(fx_disp_window,FXVerticalFrame,fx_disp_window_map,ARRAYNUMBER(fx_disp_window_map));

fx_disp_window::fx_disp_window(disp_t *d, FXComposite* p, FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs)
: FXVerticalFrame(p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs), disp(d)
{
}

void fx_disp_window::setup()
{
    setup_name();
    setup_dialog();
}

void fx_disp_window::reload()
{
    for (FXWindow *w = this->getFirst(); w; w = this->getFirst()) {
        delete w;
    }
    this->setup();
    this->create();
}

void fx_disp_window::setup_name()
{
    regress_pro *app = (regress_pro *) getApp();

    FXHorizontalFrame *modelfm = new FXHorizontalFrame(this, LAYOUT_FILL_X|FRAME_GROOVE, 0,0,0,0,0,0,0,0);
    FXString model_name(disp->dclass->full_name);
    model_name.append(" Model");
    FXLabel *model_label = new FXLabel(modelfm, model_name, NULL, LABEL_NORMAL|LAYOUT_FILL_X, 0, 0, 0, 0, 2*DEFAULT_PAD, 2*DEFAULT_PAD, 3*DEFAULT_PAD, 3*DEFAULT_PAD);
    model_label->setFont(&app->big_web_font);
    model_label->setTextColor(app->blue_web);
    model_label->setBackColor(FXRGB(255, 206, 91));

    FXHorizontalFrame *namehf = new FXHorizontalFrame(this, LAYOUT_FILL_X);
    new FXLabel(namehf, "Name ");
    FXTextField *tf = new FXTextField(namehf, 24, this, ID_NAME, FRAME_SUNKEN);
    tf->setText(CSTR(disp->name));
}

long
fx_disp_window::on_cmd_value(FXObject*, FXSelector sel, void *data)
{
    double *pvalue = this->map_parameter(FXSELID(sel) - ID_PARAM_0);
    double new_value = strtod((FXchar*)data, NULL);
    *pvalue = new_value;
    return 1;
}

long
fx_disp_window::on_changed_name(FXObject *, FXSelector, void *text)
{
    str_copy_c(disp->name, (FXchar *) text);
    return 1;
}

long
fx_disp_window::on_disp_element_add(FXObject*, FXSelector, void *)
{
    this->add_dispersion_element();
    return 1;
}

long
fx_disp_window::on_disp_element_delete(FXObject*, FXSelector sel, void *)
{
    int index = FXSELID(sel) - ID_DISP_ELEMENT_DELETE;
    this->delete_dispersion_element(index);
    return 1;
}

static void update_textfield(fx_numeric_field *tf, fx_disp_window *disp_win, int param_id)
{
    FXString vstr;
    double *pvalue = disp_win->map_parameter(param_id);
    vstr.format("%g", *pvalue);
    tf->setText(vstr);
}

static fx_numeric_field *create_textfield(FXComposite *frame, fx_disp_window *target, FXSelector id)
{
    fx_numeric_field *tf = new fx_numeric_field(frame, 8, target, id, FRAME_SUNKEN|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
    update_textfield(tf, target, id - fx_disp_window::ID_PARAM_0);
    return tf;
}

void fx_disp_ho_window::setup_dialog()
{
    FXScrollWindow *scroll_window = new FXScrollWindow(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    vframe = new FXVerticalFrame(scroll_window, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    matrix = new FXMatrix(vframe, 6, LAYOUT_SIDE_TOP|MATRIX_BY_COLUMNS);
    new FXLabel(matrix, "");
    new FXLabel(matrix, "Nosc");
    new FXLabel(matrix, "En");
    new FXLabel(matrix, "Eg");
    new FXLabel(matrix, "Nu");
    new FXLabel(matrix, "Phi");

    for (int i = 0; i < disp->disp.ho.nb_hos; i++) {
        FXButton *db = new FXButton(matrix, "", regressProApp()->delete_icon, this, ID_DISP_ELEMENT_DELETE + i, FRAME_SUNKEN);
        if (disp->disp.ho.nb_hos == 1) { db->disable(); }
        for (int j = 5*i; j < 5*(i+1); j++) {
            create_textfield(matrix, this, ID_PARAM_0 + j);
        }
    }
    new FXButton(vframe, "", regressProApp()->add_icon, this, ID_DISP_ELEMENT_ADD, FRAME_SUNKEN);
}

void fx_disp_ho_window::add_dispersion_element()
{
    int n = disp->disp.ho.nb_hos;
    disp_add_ho(disp);
    FXButton *db = new FXButton(matrix, "", regressProApp()->delete_icon, this, ID_DISP_ELEMENT_DELETE + n, FRAME_SUNKEN);
    db->create();
    for (int j = 5*n; j < 5*(n+1); j++) {
        FXTextField *tf = create_textfield(matrix, this, ID_PARAM_0 + j);
        tf->create();
    }
    matrix->childAtRowCol(1, 0)->enable();
    vframe->recalc();
}

void fx_disp_ho_window::delete_dispersion_element(int index)
{
    disp_delete_ho(disp, index);
    reload();
}

// Map
FXDEFMAP(fx_disp_fb_window) fx_disp_fb_window_map[]= {
    FXMAPFUNC(SEL_COMMAND, fx_disp_fb_window::ID_COEFF_FORM, fx_disp_fb_window::on_cmd_coeff_form),
};

FXIMPLEMENT(fx_disp_fb_window,fx_disp_window,fx_disp_fb_window_map,ARRAYNUMBER(fx_disp_fb_window_map));

void fx_disp_fb_window::setup_dialog()
{
    FXScrollWindow *scroll_window = new FXScrollWindow(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    vframe = new FXVerticalFrame(scroll_window, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    static const char *fb_parameter_names[] = {"N(inf)", "Eg", "A", "B", "C"};
    static const char *tl_parameter_names[] = {"Eps(inf)", "Eg", "AL", "E0", "C"};

    const char **pname = (disp->type == DISP_FB ? fb_parameter_names : tl_parameter_names);

    FXHorizontalFrame *coeff_frame = new FXHorizontalFrame(vframe, LAYOUT_FILL_X);
    new FXLabel(coeff_frame, "Parametrization");
    FXListBox *coeff_box = new FXListBox(coeff_frame, this, ID_COEFF_FORM, LISTBOX_NORMAL|FRAME_SUNKEN);
    coeff_box->setNumVisible(2);
    coeff_box->appendItem("Standard");
    coeff_box->appendItem("Peak-based");
    coeff_box->setCurrentItem(this->disp->disp.fb.form);

    FXHorizontalFrame *ninf_frame = new FXHorizontalFrame(vframe, LAYOUT_FILL_X);
    new FXLabel(ninf_frame, pname[0]);
    create_textfield(ninf_frame, this, ID_PARAM_0);
    new FXLabel(ninf_frame, pname[1]);
    create_textfield(ninf_frame, this, ID_PARAM_0 + 1);

    matrix = new FXMatrix(vframe, 4, LAYOUT_SIDE_TOP|MATRIX_BY_COLUMNS);
    new FXLabel(matrix, "");
    new FXLabel(matrix, pname[2]);
    new FXLabel(matrix, pname[3]);
    new FXLabel(matrix, pname[4]);

    for (int i = 0; i < disp->disp.fb.n; i++) {
        FXButton *db = new FXButton(matrix, "", regressProApp()->delete_icon, this, ID_DISP_ELEMENT_DELETE + i, FRAME_SUNKEN);
        if (disp->disp.fb.n == 1) { db->disable(); }
        for (int j = 3*i; j < 3*(i+1); j++) {
            create_textfield(matrix, this, ID_PARAM_0 + j + 2);
        }
    }
    new FXButton(vframe, "", regressProApp()->add_icon, this, ID_DISP_ELEMENT_ADD, FRAME_SUNKEN);
}

void fx_disp_fb_window::add_dispersion_element()
{
    int n = disp->disp.fb.n;
    disp_add_osc(disp);
    FXButton *db = new FXButton(matrix, "", regressProApp()->delete_icon, this, ID_DISP_ELEMENT_DELETE + n, FRAME_SUNKEN);
    db->create();
    for (int j = 3*n; j < 3*(n+1); j++) {
        FXTextField *tf = create_textfield(matrix, this, ID_PARAM_0 + j + 2);
        tf->create();
    }
    matrix->childAtRowCol(1, 0)->enable();
    vframe->recalc();
}

void fx_disp_fb_window::delete_dispersion_element(int index)
{
    disp_delete_osc(disp, index);
    reload();
}

long fx_disp_fb_window::on_cmd_coeff_form(FXObject *, FXSelector, void *data)
{
    disp_fb_change_form(this->disp, (FXival) data);
    for (int i = 0; i < disp->disp.fb.n; i++) {
        for (int j = 0; j < 3; j++) {
            fx_numeric_field *tf = (fx_numeric_field *) matrix->childAtRowCol(i + 1, j + 1);
            update_textfield(tf, this, 3 * i + j + 2);
        }
    }
    return 1;
}

void fx_disp_cauchy_window::setup_dialog()
{
    FXMatrix *matrix = new FXMatrix(this, 2, LAYOUT_SIDE_TOP|MATRIX_BY_COLUMNS);
    new FXLabel(matrix, "N");
    new FXLabel(matrix, "K");

    for (int p = 0; p < 6; p++) {
        int i = p / 2, j = p % 2;
        create_textfield(matrix, this, ID_PARAM_0 + i + 3 * j);
    }
}

fx_disp_window *new_disp_window(disp_t *d, FXComposite *comp)
{
    const FXint opts = LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_NONE;
    fx_disp_window *dispwin;
    if (d->type == DISP_HO) {
        dispwin = new fx_disp_ho_window(d, comp, opts);
    } else if (d->type == DISP_CAUCHY) {
        dispwin = new fx_disp_cauchy_window(d, comp, opts);
    } else if (d->type == DISP_LOOKUP) {
        dispwin = new fx_disp_lookup_window(d, comp, opts);
    } else if (d->type == DISP_TABLE || d->type == DISP_SAMPLE_TABLE) {
        dispwin = new fx_disp_table_window(d, comp, opts);
    } else if (d->type == DISP_FB) {
        dispwin = new fx_disp_fb_window(d, comp, opts);
    } else if (d->type == DISP_TAUC_LORENTZ) {
        dispwin = new fx_disp_fb_window(d, comp, opts);
    } else {
        dispwin = new fx_disp_window(d, comp, opts);
    }
    dispwin->setup();
    return dispwin;
}

// Map
FXDEFMAP(fx_disp_lookup_window) fx_disp_lookup_window_map[]= {
    FXMAPFUNCS(SEL_CHANGED, fx_disp_lookup_window::ID_COMPONENT_NAME, fx_disp_lookup_window::ID_COMPONENT_NAME_LAST, fx_disp_lookup_window::on_changed_component_name),
    FXMAPFUNCS(SEL_LEFTBUTTONPRESS, fx_disp_lookup_window::ID_MENU_COMPONENT, fx_disp_lookup_window::ID_MENU_COMPONENT_LAST, fx_disp_lookup_window::on_button_menu_component),
    FXMAPFUNC(SEL_COMMAND, fx_disp_lookup_window::ID_DELETE_COMP, fx_disp_lookup_window::on_cmd_delete_comp),
    FXMAPFUNC(SEL_COMMAND, fx_disp_lookup_window::ID_INSERT_COMP, fx_disp_lookup_window::on_cmd_insert_comp),
    FXMAPFUNC(SEL_COMMAND, fx_disp_lookup_window::ID_EDIT_COMP, fx_disp_lookup_window::on_cmd_edit_comp),
    FXMAPFUNC(SEL_COMMAND, fx_disp_lookup_window::ID_REPLACE_COMP, fx_disp_lookup_window::on_cmd_replace_comp),
};

FXIMPLEMENT(fx_disp_lookup_window,fx_disp_window,fx_disp_lookup_window_map,ARRAYNUMBER(fx_disp_lookup_window_map));

fx_disp_lookup_window::fx_disp_lookup_window(disp_t *d, FXComposite *p, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : fx_disp_window(d, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{
    popupmenu = new FXMenuPane(this);
    new FXMenuCommand(popupmenu,"Remove Component", NULL, this, ID_DELETE_COMP);
    new FXMenuCommand(popupmenu,"Replace Component", NULL, this, ID_REPLACE_COMP);
    new FXMenuCommand(popupmenu,"Insert Component", NULL, this, ID_INSERT_COMP);
    new FXMenuCommand(popupmenu,"Edit Component", NULL, this, ID_EDIT_COMP);
}

fx_disp_lookup_window::~fx_disp_lookup_window()
{
    delete popupmenu;
}

double *fx_disp_lookup_window::map_parameter(int index)
{
    int nat = disp_get_number_of_params(disp);
    if (index < nat) {
        return fx_disp_window::map_parameter(index);
    }
    index -= nat;
    if (index < disp->disp.lookup.nb_comps) {
        return &disp->disp.lookup.component[index].p;
    }
    return NULL;
}

void fx_disp_lookup_window::create()
{
    fx_disp_window::create();
    popupmenu->create();
}

void fx_disp_lookup_window::add_matrix_component(int index, bool create)
{
    FXString istr;
    istr.format("%d", index + 1);
    FXButton *db = new FXButton(matrix, istr, NULL, this, ID_MENU_COMPONENT + index, FRAME_SUNKEN);
    FXTextField *tf1 = new FXTextField(matrix, 24, this, ID_COMPONENT_NAME + index, FRAME_SUNKEN);
    tf1->setText(CSTR(disp->disp.lookup.component[index].disp->name));
    FXTextField *tf2 = create_textfield(matrix, this, ID_PARAM_0 + 1 + index);
    if (create) {
        db->create();
        tf1->create();
        tf2->create();
    }
}

void fx_disp_lookup_window::setup_dialog()
{
    regress_pro *app = regressProApp();

    FXScrollWindow *scroll_window = new FXScrollWindow(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    vframe = new FXVerticalFrame(scroll_window, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    FXHorizontalFrame *thf = new FXHorizontalFrame(vframe);
    new FXLabel(thf, "P");
    create_textfield(thf, this, ID_PARAM_0);

    matrix = new FXMatrix(vframe, 3, LAYOUT_SIDE_TOP|MATRIX_BY_COLUMNS);
    new FXLabel(matrix, "");
    FXLabel *h1 = new FXLabel(matrix, "-- Dispersion --");
    h1->setFont(&app->small_font);
    h1->setTextColor(app->blue_highlight);
    FXLabel *h2 = new FXLabel(matrix, "-- P --");
    h2->setFont(&app->small_font);
    h2->setTextColor(app->blue_highlight);

    for (int i = 0; i < disp->disp.lookup.nb_comps; i++) {
        add_matrix_component(i);
    }
    new FXButton(vframe, "", regressProApp()->add_icon, this, ID_DISP_ELEMENT_ADD, FRAME_SUNKEN);
}

void fx_disp_lookup_window::add_dispersion_element()
{
    disp_t *comp = ui_choose_dispersion(this);
    if (!comp) return;
    disp_lookup *lookup = &disp->disp.lookup;
    int n = lookup->nb_comps;
    double p1 = lookup->component[0].p, p2 = lookup->component[n-1].p;
    double p0 = p2 + (n > 1 ? (p2 - p1) / (n - 1) : 1.0);
    disp_lookup_add_comp(disp, n, comp, p0);
    add_matrix_component(n, true);
    vframe->recalc();
}

long fx_disp_lookup_window::on_cmd_delete_comp(FXObject *, FXSelector, void *)
{
    disp_lookup_delete_comp(disp, selected_component);
    reload();
    return 1;
}

long fx_disp_lookup_window::on_cmd_insert_comp(FXObject *, FXSelector, void *)
{
    disp_t *comp = ui_choose_dispersion(this);
    if (!comp) return 1;
    disp_lookup *lookup = &disp->disp.lookup;
    const int i = selected_component;
    const int n = lookup->nb_comps;
    double new_p;
    if (i > 0) {
        double p1 = lookup->component[i-1].p, p2 = lookup->component[i].p;
        new_p = (p1 + p2) / 2.0;
    } else {
        double p1 = lookup->component[0].p, p2 = lookup->component[n-1].p;
        new_p = p1 - (n > 1 ? (p2 - p1) / (n - 1) : 1.0);
    }
    disp_lookup_add_comp(disp, i, comp, new_p);
    reload();
    return 1;
}

long fx_disp_lookup_window::on_cmd_edit_comp(FXObject *, FXSelector, void *)
{
    const int i = selected_component;
    disp_lookup *lookup = &disp->disp.lookup;
    disp_t *new_disp = ui_edit_dispersion(this, lookup->component[i].disp);
    if (new_disp) {
        lookup->component[i].disp = new_disp;
        FXTextField *tf = (FXTextField *) matrix->childAtRowCol(i + 1, 1);
        tf->setText(CSTR(new_disp->name));
    }
    return 1;
}

long fx_disp_lookup_window::on_cmd_replace_comp(FXObject *, FXSelector, void *)
{
    const int i = selected_component;
    disp_lookup *lookup = &disp->disp.lookup;
    disp_t *new_disp = ui_choose_dispersion(this);
    if (!new_disp) return 1;
    disp_free(lookup->component[i].disp);
    lookup->component[i].disp = new_disp;
    FXTextField *tf = (FXTextField *) matrix->childAtRowCol(i + 1, 1);
    tf->setText(CSTR(new_disp->name));
    return 1;
}

long fx_disp_lookup_window::on_changed_component_name(FXObject *, FXSelector sel, void *text)
{
    int index = FXSELID(sel) - ID_COMPONENT_NAME;
    disp_lookup *lookup = &disp->disp.lookup;
    str_copy_c(lookup->component[index].disp->name, (FXchar *)text);
    return 1;
}

long
fx_disp_lookup_window::on_button_menu_component(FXObject*sender, FXSelector sel, void *ptr)
{
    FXEvent *event = (FXEvent *) ptr;
    selected_component = FXSELID(sel) - ID_MENU_COMPONENT;
    if(!event->moved){
        popupmenu->popup(NULL, event->root_x, event->root_y);
        getApp()->runModalWhileShown(popupmenu);
    }
    return 1;
}

void fx_disp_table_window::setup_dialog()
{
    int rows = m_iterator->rows();

    m_table = new FXTable(this, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y|TABLE_READONLY);
    m_table->setTableSize(rows, 3);

    m_table->setRowHeaderWidth(0);
    m_table->setColumnText(0, "Wavelength (nm)");
    m_table->setColumnText(1, "n");
    m_table->setColumnText(2, "k");

    for (int i = 0; i < rows; i++) {
        float n, k;
        float wl = m_iterator->get_nk_row(i, &n, &k);
        m_table->setItemText(i, 0, FXStringFormat("%g", wl));
        m_table->setItemText(i, 1, FXStringFormat("%g", n));
        m_table->setItemText(i, 2, FXStringFormat("%g", k));
    }
}
