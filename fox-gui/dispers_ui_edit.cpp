#include <algorithm>

#include "dispers_ui_edit.h"
#include "disp-ho.h"
#include "disp-fb.h"
#include "dispers_ui_utils.h"
#include "dispers-library.h"
#include "regress_pro.h"
#include "disp-lookup-components.h"
#include "disp-bruggeman-components.h"

// Map
FXDEFMAP(fx_disp_window) fx_disp_window_map[]= {
    FXMAPFUNC(SEL_CHANGED,  fx_disp_window::ID_NAME, fx_disp_window::on_changed_name),
    FXMAPFUNC(SEL_CHANGED,  fx_disp_window::ID_RANGE,               fx_disp_window::on_changed_range),
    FXMAPFUNC(SEL_CHANGED,  fx_disp_window::ID_DESCRIPTION,         fx_disp_window::on_changed_description),
    FXMAPFUNC(SEL_UPDATE,   fx_disp_window::ID_RANGE,               fx_disp_window::on_update_range),
    FXMAPFUNC(SEL_UPDATE,   fx_disp_window::ID_DESCRIPTION,         fx_disp_window::on_update_description),
    FXMAPFUNC(SEL_COMMAND,  fx_disp_window::ID_DISP_ELEMENT_ADD,    fx_disp_window::on_disp_element_add),
    FXMAPFUNCS(SEL_COMMAND, fx_disp_window::ID_DISP_ELEMENT_DELETE, fx_disp_window::ID_DISP_ELEMENT_DELETE_LAST, fx_disp_window::on_disp_element_delete),
    FXMAPFUNCS(SEL_CHANGED, fx_disp_ho_window::ID_PARAM_0,          fx_disp_ho_window::ID_PARAM_LAST, fx_disp_ho_window::on_cmd_value),
    FXMAPFUNC(SEL_COMMAND,  fx_disp_window::ID_CLEAR_FLAG,          fx_disp_window::on_cmd_clear_flag),
    FXMAPFUNC(SEL_COMMAND,  fx_disp_window::ID_DESCRIPTION_TOGGLE,  fx_disp_window::on_cmd_description_toggle),
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

FXWindow *fx_disp_window::new_description_area(FXComposite *container, const char *text) {
    auto frame = new FXPacker(container, FRAME_LINE | LAYOUT_FILL_X| LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0);
    frame->setBorderColor(FXRGB(197,213,221));
    auto textfield = new FXText(frame, this, ID_DESCRIPTION, TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    textfield->setFont(&regressProApp()->lit_font);
    textfield->setText(text);
    const FXint nrows = textfield->getNumRows();
    textfield->setVisibleRows(std::min(nrows,3));
    textfield->setVisibleColumns(60);
    textfield->setBackColor(FXRGB(230,241,246));
    textfield->setTipText("Description of the dispersion. Enter a new description or modify the content.");
    return frame;
}

void fx_disp_window::setup_name()
{
    regress_pro *app = (regress_pro *) getApp();

    FXHorizontalFrame *modelfm = new FXHorizontalFrame(this, LAYOUT_FILL_X|FRAME_GROOVE, 0,0,0,0, 0,0,0,0, 0,0);
    FXString model_name(disp->dclass->full_name);
    model_name.append(" Model");
    FXLabel *model_label = new FXLabel(modelfm, model_name, NULL, LABEL_NORMAL|LAYOUT_FILL_X, 0, 0, 0, 0, 2*DEFAULT_PAD, 2*DEFAULT_PAD, 3*DEFAULT_PAD, 3*DEFAULT_PAD);
    model_label->setFont(&app->big_web_font);
    model_label->setTextColor(app->blue_web);
    model_label->setBackColor(FXRGB(255, 206, 91));

    FXHorizontalFrame *namehf = new FXHorizontalFrame(this, LAYOUT_FILL_X);
    new FXLabel(namehf, "Name ");
    FXTextField *tf = new FXTextField(namehf, 16, this, ID_NAME, FRAME_SUNKEN);
    tf->setText(disp_get_name(disp));

    FXHorizontalFrame *range_hf = namehf;
    new FXLabel(range_hf, "Range ");
    range_start_textfield = new fx_numeric_field(range_hf, 8, this, ID_RANGE, FRAME_SUNKEN|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
    range_end_textfield   = new fx_numeric_field(range_hf, 8, this, ID_RANGE,   FRAME_SUNKEN|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
    double wavelength_start, wavelength_end;
    int samples_number;
    disp_get_wavelength_range(disp, &wavelength_start, &wavelength_end, &samples_number);
    if (disp_is_tabular(disp) || DISP_VALID_RANGE(disp->info->wavelength_start, disp->info->wavelength_end)) {
        range_start_textfield->setNumber(wavelength_start);
        range_end_textfield->setNumber(wavelength_end);
    }
    set_range_color();
    if (disp_is_tabular(disp)) {
        range_start_textfield->disable();
        range_end_textfield->disable();
    }

    auto descr_group = new FXVerticalFrame(this, FRAME_NONE | LAYOUT_FILL_X, 0, 0, 0, 0, 0, 0, 0, 0);

    m_description_frame = new FXVerticalFrame(descr_group, FRAME_NONE | LAYOUT_FILL_X | LAYOUT_FILL_Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    auto descr_label_frame = new FXHorizontalFrame(m_description_frame, FRAME_NONE, 0, 0, 0, 0, 0, 0, 0, 0);
    m_description_button = new FXButton(descr_label_frame, ">", NULL, this, ID_DESCRIPTION_TOGGLE, BUTTON_TOOLBAR, 0, 0, 0, 0, 0, 0, 0, 0);
    m_description_button->setFont(&app->monospace_font);
    new FXLabel(descr_label_frame, "Description", NULL, LABEL_NORMAL);

    if (str_is_null(disp->info->description)) {
        m_description_widget = nullptr;
    } else {
        m_description_widget = new_description_area(m_description_frame, CSTR(disp->info->description));
        m_description_button->setText("v");
    }

    if (disp->info->modifications_stamp) {
        m_message_frame = new FXHorizontalFrame(descr_group, LAYOUT_FILL_X | FRAME_NONE, 0, 0, 0, 0, 0, 0, 0, 0);
        new FXButton(m_message_frame, "", app->broom_icon, this, ID_CLEAR_FLAG, BUTTON_TOOLBAR, 0, 0, 0, 0, 0, 0, 0, 0);
        auto label = new FXLabel(m_message_frame, CSTR(disp->info->modifications_stamp), NULL, LABEL_NORMAL);
        label->setFont(&app->small_font);
        label->setTextColor(app->blue_highlight);
    }
}

long
fx_disp_window::on_cmd_description_toggle(FXObject*, FXSelector sel, void *data)
{
    if (m_description_widget) {
        m_description_button->setText(">");
        delete m_description_widget;
        m_description_widget = nullptr;
    } else {
        m_description_button->setText("v");
        m_description_widget = new_description_area(m_description_frame, CSTR(disp->info->description));
        m_description_widget->create();
    }
    recalc();
    return 1;
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
fx_disp_window::on_changed_description(FXObject *, FXSelector, void *)
{
    FXText *description_textfield = (FXText *) m_description_widget->getFirst();
    FXint length = description_textfield->getLength();
    STR_SIZE_CHECK(disp->info->description, (size_t)length);
    description_textfield->getText((FXchar *) disp->info->description->heap, length);
    disp->info->description->heap[length] = 0;
    disp->info->description->length = length;
    this->handle(this, FXSEL(SEL_UPDATE, ID_DESCRIPTION), NULL);
    return 1;
}

long
fx_disp_window::on_update_description(FXObject *, FXSelector, void *)
{
    FXText *description_textfield = (FXText *) m_description_widget->getFirst();
    FXint nrows = std::min(description_textfield->getNumRows(), 3);
    FXint nvisi = std::min(description_textfield->getVisibleRows(), 3);
    if (nrows != nvisi) {
        description_textfield->setVisibleRows(nrows);
        recalc();
        return 1;
    }
    return 0;
}

long
fx_disp_window::on_changed_name(FXObject *, FXSelector, void *text)
{
    disp_set_name(disp, (FXchar *) text);
    return 1;
}

bool
fx_disp_window::range_is_valid() const {
    double wavelength_start = range_start_textfield->getNumber();
    double wavelength_end   = range_end_textfield  ->getNumber();
    return DISP_VALID_RANGE(wavelength_start, wavelength_end);
}

void
fx_disp_window::set_range_color() {
    FX::FXColor color = (range_is_valid() ? regressProApp()->black : regressProApp()->red_warning);
    range_start_textfield->setTextColor(color);
    range_end_textfield  ->setTextColor(color);
}

long
fx_disp_window::on_update_range(FXObject *obj, FXSelector, void *text) {
    set_range_color();
    return 1;
}

long
fx_disp_window::on_changed_range(FXObject *obj, FXSelector, void *text)
{
    if (!range_is_valid()) return 1;
    disp->info->wavelength_start = range_start_textfield->getNumber();
    disp->info->wavelength_end   = range_end_textfield  ->getNumber();
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

static fx_numeric_field *create_textfield(FXComposite *frame, fx_disp_window *target, FXSelector id, int textfield_size = 8)
{
    fx_numeric_field *tf = new fx_numeric_field(frame, textfield_size, target, id, FRAME_SUNKEN|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
    update_textfield(tf, target, id - fx_disp_window::ID_PARAM_0);
    return tf;
}

void fx_disp_ho_window::setup_dialog()
{
    const int extra_params_no = 3;
    FXScrollWindow *scroll_window = new FXScrollWindow(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    vframe = new FXVerticalFrame(scroll_window, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    FXHorizontalFrame *thf = new FXHorizontalFrame(vframe);
    new FXLabel(thf, "Eps(inf)");
    create_textfield(thf, this, ID_PARAM_0);
    new FXLabel(thf, "Eps(host)");
    create_textfield(thf, this, ID_PARAM_0 + 1);
    new FXLabel(thf, "Nu(host)");
    create_textfield(thf, this, ID_PARAM_0 + 2);

    matrix = new FXMatrix(vframe, 6, LAYOUT_SIDE_TOP|MATRIX_BY_COLUMNS);
    new FXLabel(matrix, "");
    new FXLabel(matrix, "Nosc");
    new FXLabel(matrix, "En");
    new FXLabel(matrix, "Eg");
    new FXLabel(matrix, "Nu");
    new FXLabel(matrix, "Phi");

    const int osc_params_no = disp_ho_oscillator_parameters_number(disp);
    for (int i = 0; i < disp->disp.ho.nb_hos; i++) {
        FXButton *db = new FXButton(matrix, "", regressProApp()->delete_icon, this, ID_DISP_ELEMENT_DELETE + i, FRAME_SUNKEN);
        if (disp->disp.ho.nb_hos == 1) { db->disable(); }
        for (int j = osc_params_no*i; j < osc_params_no*(i+1); j++) {
            create_textfield(matrix, this, ID_PARAM_0 + extra_params_no + j);
        }
    }
    new FXButton(vframe, "", regressProApp()->add_icon, this, ID_DISP_ELEMENT_ADD, FRAME_SUNKEN);
}

void fx_disp_ho_window::add_dispersion_element()
{
    const int extra_params_no = 3;
    const int n = disp->disp.ho.nb_hos;
    disp_add_ho(disp);
    FXButton *db = new FXButton(matrix, "", regressProApp()->delete_icon, this, ID_DISP_ELEMENT_DELETE + n, FRAME_SUNKEN);
    db->create();
    const int osc_params_no = disp_ho_oscillator_parameters_number(disp);
    for (int j = osc_params_no*n; j < osc_params_no*(n+1); j++) {
        FXTextField *tf = create_textfield(matrix, this, ID_PARAM_0 + extra_params_no + j);
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
FXDEFMAP(fx_disp_lorentz_window) fx_disp_lorentz_window_map[]= {
    FXMAPFUNC(SEL_COMMAND, fx_disp_lorentz_window::ID_COEFF_FORM, fx_disp_lorentz_window::on_cmd_coeff_form),
};

FXIMPLEMENT(fx_disp_lorentz_window,fx_disp_window,fx_disp_lorentz_window_map,ARRAYNUMBER(fx_disp_lorentz_window_map));

void fx_disp_lorentz_window::setup_dialog()
{
    FXScrollWindow *scroll_window = new FXScrollWindow(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    vframe = new FXVerticalFrame(scroll_window, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    FXHorizontalFrame *coeff_frame = new FXHorizontalFrame(vframe, LAYOUT_FILL_X);
    new FXLabel(coeff_frame, "Parametrization");
    FXListBox *coeff_box = new FXListBox(coeff_frame, this, ID_COEFF_FORM, LISTBOX_NORMAL|FRAME_SUNKEN);
    coeff_box->setNumVisible(2);
    coeff_box->appendItem("A Br En");
    coeff_box->appendItem("A En^2");
    coeff_box->setCurrentItem(this->disp->disp.lorentz.style);

    FXHorizontalFrame *thf = new FXHorizontalFrame(vframe);
    new FXLabel(thf, "Eps(inf)");
    create_textfield(thf, this, ID_PARAM_0);

    matrix = new FXMatrix(vframe, 4, LAYOUT_SIDE_TOP|MATRIX_BY_COLUMNS);
    new FXLabel(matrix, "");
    new FXLabel(matrix, "A");
    new FXLabel(matrix, "En");
    new FXLabel(matrix, "Br");

    const int osc_params_no = disp_lorentz_oscillator_parameters_number(disp);
    for (int i = 0; i < disp->disp.lorentz.oscillators_number; i++) {
        FXButton *db = new FXButton(matrix, "", regressProApp()->delete_icon, this, ID_DISP_ELEMENT_DELETE + i, FRAME_SUNKEN);
        if (disp->disp.lorentz.oscillators_number == 1) { db->disable(); }
        for (int j = osc_params_no*i; j < osc_params_no*(i+1); j++) {
            create_textfield(matrix, this, ID_PARAM_0 + j + 1);
        }
    }
    new FXButton(vframe, "", regressProApp()->add_icon, this, ID_DISP_ELEMENT_ADD, FRAME_SUNKEN);
}

void fx_disp_lorentz_window::add_dispersion_element()
{
    const int n = disp->disp.lorentz.oscillators_number;
    disp_lorentz_add_oscillator(disp);
    FXButton *db = new FXButton(matrix, "", regressProApp()->delete_icon, this, ID_DISP_ELEMENT_DELETE + n, FRAME_SUNKEN);
    db->create();
    const int osc_params_no = disp_lorentz_oscillator_parameters_number(disp);
    for (int j = osc_params_no*n; j < osc_params_no*(n+1); j++) {
        FXTextField *tf = create_textfield(matrix, this, ID_PARAM_0 + j + 1);
        tf->create();
    }
    matrix->childAtRowCol(1, 0)->enable();
    vframe->recalc();
}

void fx_disp_lorentz_window::delete_dispersion_element(int index)
{
    disp_lorentz_delete_oscillator(disp, index);
    reload();
}

long fx_disp_lorentz_window::on_cmd_coeff_form(FXObject *, FXSelector, void *data)
{
    disp_lorentz_change_style(this->disp, (FXival) data);
    const int pposc = disp_lorentz_oscillator_parameters_number(this->disp);
    for (int i = 0; i < disp->disp.lorentz.oscillators_number; i++) {
        for (int j = 0; j < pposc; j++) {
            fx_numeric_field *tf = (fx_numeric_field *) matrix->childAtRowCol(i + 1, j + 1);
            update_textfield(tf, this, pposc * i + j + 1);
        }
    }
    return 1;
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
    FXMatrix *matrix = new FXMatrix(this, 2, LAYOUT_SIDE_TOP|MATRIX_BY_COLUMNS, 0,0,0,0, 8*DEFAULT_SPACING,8*DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING, 6*DEFAULT_SPACING,DEFAULT_SPACING);
    new FXLabel(matrix, disp->type == DISP_CAUCHY ? "N" : "A", NULL, LAYOUT_CENTER_X);
    new FXLabel(matrix, disp->type == DISP_CAUCHY ? "K" : "B (µm^2)", NULL, LAYOUT_CENTER_X);

    for (int p = 0; p < 6; p++) {
        int i = p / 2, j = p % 2;
        create_textfield(matrix, this, ID_PARAM_0 + i + 3 * j, 14);
    }
}

fx_disp_window *new_disp_window(disp_t *d, FXComposite *comp)
{
    const FXint opts = LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_NONE;
    fx_disp_window *dispwin;
    if (d->type == DISP_HO) {
        dispwin = new fx_disp_ho_window(d, comp, opts);
    } else if (d->type == DISP_CAUCHY || d->type == DISP_SELLMEIER) {
        dispwin = new fx_disp_cauchy_window(d, comp, opts);
    } else if (d->type == DISP_LOOKUP) {
        dispwin = new fx_lookup_window(d, comp, opts);
    } else if (d->type == DISP_BRUGGEMAN) {
        dispwin = new fx_bruggeman_window(d, comp, opts);
    } else if (d->type == DISP_TABLE || d->type == DISP_SAMPLE_TABLE) {
        dispwin = new fx_disp_table_window(d, comp, opts);
    } else if (d->type == DISP_FB) {
        dispwin = new fx_disp_fb_window(d, comp, opts);
    } else if (d->type == DISP_TAUC_LORENTZ) {
        dispwin = new fx_disp_fb_window(d, comp, opts);
    } else if (d->type == DISP_LORENTZ) {
        dispwin = new fx_disp_lorentz_window(d, comp, opts);
    } else {
        dispwin = new fx_disp_window(d, comp, opts);
    }
    dispwin->setup();
    return dispwin;
}

// Map
FXDEFMAP(fx_components_window) fx_components_window_map[]= {
    FXMAPFUNCS(SEL_CHANGED, fx_components_window::ID_COMPONENT_NAME, fx_components_window::ID_COMPONENT_NAME_LAST, fx_components_window::on_changed_component_name),
    FXMAPFUNCS(SEL_LEFTBUTTONPRESS, fx_components_window::ID_MENU_COMPONENT, fx_components_window::ID_MENU_COMPONENT_LAST, fx_components_window::on_button_menu_component),
    FXMAPFUNC(SEL_COMMAND, fx_components_window::ID_DELETE_COMP, fx_components_window::on_cmd_delete_comp),
    FXMAPFUNC(SEL_COMMAND, fx_components_window::ID_INSERT_COMP, fx_components_window::on_cmd_insert_comp),
    FXMAPFUNC(SEL_COMMAND, fx_components_window::ID_EDIT_COMP, fx_components_window::on_cmd_edit_comp),
    FXMAPFUNC(SEL_COMMAND, fx_components_window::ID_REPLACE_COMP, fx_components_window::on_cmd_replace_comp),
    FXMAPFUNC(SEL_COMMAND, fx_components_window::ID_SAVE_USERLIB, fx_components_window::on_cmd_save_userlib),
};

FXIMPLEMENT(fx_components_window,fx_disp_window,fx_components_window_map,ARRAYNUMBER(fx_components_window_map));

fx_components_window::fx_components_window(disp_t *d, disp_components *components, FXComposite *p, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : fx_disp_window(d, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs), m_components(components)
{
    popupmenu = new FXMenuPane(this);
    new FXMenuCommand(popupmenu,"Remove Component", NULL, this, ID_DELETE_COMP);
    new FXMenuCommand(popupmenu,"Replace Component", NULL, this, ID_REPLACE_COMP);
    new FXMenuCommand(popupmenu,"Insert Component", NULL, this, ID_INSERT_COMP);
    new FXMenuCommand(popupmenu,"Edit Component", NULL, this, ID_EDIT_COMP);
    new FXMenuSeparator(popupmenu);
    new FXMenuCommand(popupmenu,"Save to User Library", NULL, this, ID_SAVE_USERLIB);
}

fx_components_window::~fx_components_window()
{
    delete popupmenu;
}

double *fx_components_window::map_parameter(int index)
{
    int nat = disp_get_number_of_params(disp);
    if (index < nat) {
        return fx_disp_window::map_parameter(index);
    }
    index -= nat;
    if (index < m_components->length()) {
        return m_components->map_component_value(index);
    }
    return NULL;
}

void fx_components_window::create()
{
    fx_disp_window::create();
    popupmenu->create();
}

void fx_components_window::add_matrix_component(int index, FXuint sel_id, bool use_textfield, bool create) {
    FXString istr;
    istr.format("%d", index + 1);
    FXButton *db = new FXButton(matrix, istr, NULL, this, ID_MENU_COMPONENT + index, FRAME_SUNKEN);
    FXTextField *tf1 = new FXTextField(matrix, 24, this, ID_COMPONENT_NAME + index, FRAME_SUNKEN);
    tf1->setText(disp_get_name(m_components->disp(index)));
    FXWindow *tf2;
    if (use_textfield) {
        tf2 = create_textfield(matrix, this, sel_id);
    } else {
        tf2 = new FXLabel(matrix, "", NULL);
    }
    if (create) {
        db->create();
        tf1->create();
        tf2->create();
    }
}

void fx_components_window::add_dispersion_element()
{
    disp_t *comp = ui_choose_dispersion(this);
    if (!comp) return;
    int index_append = m_components->length();
    m_components->insert(index_append, comp);
    add_matrix_component(index_append, get_param_id(index_append), true, true);
    vframe->recalc();
}

long fx_components_window::on_cmd_delete_comp(FXObject *, FXSelector, void *)
{
    m_components->remove(selected_component);
    reload();
    return 1;
}

long fx_components_window::on_cmd_insert_comp(FXObject *, FXSelector, void *)
{
    disp_t *comp = ui_choose_dispersion(this);
    if (!comp) return 1;
    m_components->insert(selected_component, comp);
    reload();
    return 1;
}

long fx_components_window::on_cmd_edit_comp(FXObject *, FXSelector, void *)
{
    const int i = selected_component;
    disp_t *new_disp = ui_edit_dispersion(this, m_components->disp(i));
    if (new_disp) {
        m_components->assign_disp(i, new_disp);
        FXTextField *tf = (FXTextField *) matrix->childAtRowCol(i + 1, 1);
        tf->setText(disp_get_name(m_components->disp(i)));
    }
    return 1;
}

long fx_components_window::on_cmd_replace_comp(FXObject *, FXSelector, void *)
{
    const int i = selected_component;
    disp_t *new_disp = ui_choose_dispersion(this);
    if (!new_disp) return 1;
    disp_t *old_disp = m_components->disp(i);
    m_components->assign_disp(i, new_disp);
    disp_free(old_disp);
    FXTextField *tf = (FXTextField *) matrix->childAtRowCol(i + 1, 1);
    tf->setText(disp_get_name(m_components->disp(i)));
    return 1;
}

long fx_components_window::on_cmd_save_userlib(FXObject *, FXSelector, void *)
{
    const int i = selected_component;
    disp_t *d = disp_copy(m_components->disp(i));
    disp_list_add(user_lib, d, NULL);
    return 1;
}


long fx_components_window::on_changed_component_name(FXObject *, FXSelector sel, void *text)
{
    int index = FXSELID(sel) - ID_COMPONENT_NAME;
    disp_set_name(m_components->disp(index), (FXchar *) text);
    return 1;
}

long
fx_components_window::on_button_menu_component(FXObject*sender, FXSelector sel, void *ptr)
{
    FXEvent *event = (FXEvent *) ptr;
    selected_component = FXSELID(sel) - ID_MENU_COMPONENT;
    if(!event->moved){
        popupmenu->popup(NULL, event->root_x, event->root_y);
        getApp()->runModalWhileShown(popupmenu);
    }
    return 1;
}

fx_lookup_window::fx_lookup_window(disp_t *d, FXComposite *p, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
: fx_components_window(d, new disp_lookup_components(d), p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{ }

FXuint fx_lookup_window::get_param_id(int index) { return ID_PARAM_0 + 1 + index; }

void fx_lookup_window::setup_dialog()
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

    for (int i = 0; i < m_components->length(); i++) {
        add_matrix_component(i, get_param_id(i), true);
    }
    new FXButton(vframe, "", regressProApp()->add_icon, this, ID_DISP_ELEMENT_ADD, FRAME_SUNKEN);
}

fx_bruggeman_window::fx_bruggeman_window(disp_t *d, FXComposite *p, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
: fx_components_window(d, new disp_bruggeman_components(d), p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{ }

FXuint fx_bruggeman_window::get_param_id(int index) { return ID_PARAM_0 + index - 1; }

void fx_bruggeman_window::setup_dialog()
{
    regress_pro *app = regressProApp();

    FXScrollWindow *scroll_window = new FXScrollWindow(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    vframe = new FXVerticalFrame(scroll_window, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    matrix = new FXMatrix(vframe, 3, LAYOUT_SIDE_TOP|MATRIX_BY_COLUMNS);
    new FXLabel(matrix, "");
    FXLabel *h1 = new FXLabel(matrix, "-- Dispersion --");
    h1->setFont(&app->small_font);
    h1->setTextColor(app->blue_highlight);
    FXLabel *h2 = new FXLabel(matrix, "-- Fraction --");
    h2->setFont(&app->small_font);
    h2->setTextColor(app->blue_highlight);

    for (int i = 0; i < m_components->length(); i++) {
        add_matrix_component(i, get_param_id(i), i > 0);
    }
    new FXButton(vframe, "", regressProApp()->add_icon, this, ID_DISP_ELEMENT_ADD, FRAME_SUNKEN);
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

long
fx_disp_window::on_cmd_clear_flag(FXObject*, FXSelector, void *data)
{
    if (disp->info->modifications_stamp) {
        str_free(disp->info->modifications_stamp);
        free(disp->info->modifications_stamp);
        disp->info->modifications_stamp = NULL;
    }
    delete m_message_frame;
    recalc();
    return 1;
}
