#include "fit_panel.h"

#include "fx_numeric_field.h"
#include "str_cpp.h"
#include "regress_pro.h"

// Map
FXDEFMAP(fit_panel) fit_panel_map[]= {
    FXMAPFUNC(SEL_COMMAND, fit_panel::ID_PARAM_SELECT, fit_panel::on_cmd_param_select),
    FXMAPFUNC(SEL_COMMAND, fit_panel::ID_PARAM_VALUE,  fit_panel::on_cmd_param_change),
    FXMAPFUNC(SEL_CHANGED, fit_panel::ID_PARAM_VALUE,  fit_panel::on_cmd_param_change),
    FXMAPFUNC(SEL_UPDATE,  fit_panel::ID_PARAM_VALUE,  fit_panel::on_update_param),
    FXMAPFUNC(SEL_COMMAND, fit_panel::ID_RUN_FIT,      fit_panel::on_cmd_run_fit),
    FXMAPFUNC(SEL_COMMAND, fit_panel::ID_PLOT_SCALE,   fit_panel::on_cmd_plot_autoscale),
    FXMAPFUNC(SEL_COMMAND, fit_panel::ID_SPECTR_RANGE, fit_panel::on_cmd_spectral_range),
    FXMAPFUNC(SEL_CHANGED, fit_panel::ID_SPECTR_RANGE, fit_panel::on_change_spectral_range),
    FXMAPFUNC(SEL_UPDATE,  fit_panel::ID_SPECTR_RANGE, fit_panel::on_update_spectral_range),
    FXMAPFUNC(SEL_COMMAND, fit_panel::ID_ACTION_UNDO,  fit_panel::on_cmd_undo),
    FXMAPFUNC(SEL_COMMAND, fit_panel::ID_ACTION_REDO,  fit_panel::on_cmd_undo),
    FXMAPFUNC(SEL_UPDATE,  fit_panel::ID_ACTION_UNDO,  fit_panel::on_update_undo_menu),
    FXMAPFUNC(SEL_UPDATE,  fit_panel::ID_ACTION_REDO,  fit_panel::on_update_undo_menu),
    FXMAPFUNC(SEL_COMMAND, fit_panel::ID_PLOT_COPY,    fit_panel::on_cmd_plot_copy),
    FXMAPFUNC(SEL_COMMAND, fit_panel::ID_PLOT_COPY_AS_IMAGE, fit_panel::on_cmd_plot_copy_as_image),
};

// Object implementation
FXIMPLEMENT(fit_panel,FXHorizontalFrame,fit_panel_map,ARRAYNUMBER(fit_panel_map));

fit_panel::fit_panel(fit_manager* fit, FXComposite *p, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXHorizontalFrame(p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs),
      m_canvas(0), m_fit(fit), m_results_target(nullptr)
{
    scroll_window = new FXScrollWindow(this, VSCROLLER_ALWAYS | HSCROLLING_OFF | LAYOUT_FILL_Y);
    setup();
}

void fit_panel::clear()
{
    /* Delete the top level widgets that will delete, in turns, all theirs
       child widgets. */
    delete param_matrix;
    delete m_canvas_frame;
}

void fit_panel::config_spectral_range()
{
    double wls, wle;
    m_fit->get_sampling(wls, wle);
    m_wl_entry->setText(FXString::value("%g-%g", wls, wle));
    range_dirty = false;
}

void fit_panel::set_fit_result(const lmfit_result& result)
{
    m_result_label->setText(FXString::value("Chi Square: %10.4g   points: %4d   iterations: %4d   fit status: %s", result.chisq, result.nb_points, result.nb_iterations, lmfit_result_error_string(&result)));
}

void fit_panel::setup()
{
    param_matrix = new FXMatrix(scroll_window, 2, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_Y|MATRIX_BY_COLUMNS, 0, 0, 0, 0, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, 1, 1);

    new FXLabel(param_matrix, "Range");
    m_wl_entry = new FXTextField(param_matrix, 10, this, ID_SPECTR_RANGE, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_ROW);

    config_spectral_range();

    m_parameters.resize(m_fit->parameters_number());

    str pname;
    FXString label_text;
    int current_layer = 0;
    for(unsigned k = 0; k < m_parameters.size(); k++) {
        param_info* p = this->get_parameter_pointer(k);
        m_fit->get_parameter(k, &p->fp);

        if(p->fp.id == PID_LAYER_N && p->fp.layer_nb != current_layer) {
            current_layer = p->fp.layer_nb;
            label_text.format("Layer %i", current_layer);
            FXLabel *lab = new FXLabel(param_matrix, label_text);
            lab->setFont(&regressProApp()->bold_font);
            new FXLabel(param_matrix, "");
        }

        if(p->fp.id >= PID_ACQUISITION_PARAMETER && current_layer >= 0) {
            label_text.format("Acquisition");
            FXLabel *lab = new FXLabel(param_matrix, label_text);
            lab->setFont(&regressProApp()->bold_font);
            new FXLabel(param_matrix, "");
            current_layer = -1;
        }

        get_param_name(&p->fp, &pname);
        FXString fxpname((const FXchar *) pname.text());
        FXCheckButton* bt = new FXCheckButton(param_matrix, fxpname, this, ID_PARAM_SELECT);
        FXTextField* tf = new fx_numeric_field(param_matrix, 10, this, ID_PARAM_VALUE, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);

        tf->setUserData(p);
        bt->setUserData(p);

        p->text_field = tf;
        p->selected = false;
        p->is_dirty = true;
    }

    m_undo_manager.clear();

    m_canvas_frame = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_canvas_frame->setBackColor(FXRGB(255,255,255));
    m_canvas = new plot_canvas(m_canvas_frame, nullptr, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_fit->config_plot(m_canvas);

    FXFont& bold_font = regressProApp()->bold_font;
    const FXint label_height = bold_font.getFontHeight() + DEFAULT_PAD;
    m_result_label = new FXLabel(m_canvas_frame, "", nullptr, LABEL_NORMAL|LAYOUT_FIX_HEIGHT, 0, 0, 0, label_height, DEFAULT_PAD, DEFAULT_PAD, 0, DEFAULT_PAD);
    m_result_label->setFont(&bold_font);
    m_result_label->setBackColor(FXRGB(255,255,255));
    m_result_label->setHeight(bold_font.getFontHeight() + 2 * DEFAULT_PAD);
}

void fit_panel::reload()
{
    clear();
    setup();
    /* Since the top level widgets were deleted and recreated we need to call
       "create" on them. */
    param_matrix->create();
    m_canvas_frame->create();
}

long
fit_panel::on_cmd_param_select(FXObject* _cb, FXSelector, void*)
{
    FXCheckButton *cb = (FXCheckButton *) _cb;
    param_info* p_inf = (param_info*) cb->getUserData();
    p_inf->selected = cb->getCheck();
    return 1;
}

long
fit_panel::on_update_param(FXObject *_txt, FXSelector, void*)
{
    FXTextField *txt = (FXTextField *) _txt;
    param_info* p = (param_info*) txt->getUserData();
    if(p->is_dirty) {
        unsigned k = this->get_parameter_index(p);
        FXString s = FXString::value("%g", m_fit->get_parameter_value(k));
        txt->setText(s, false);
        p->is_dirty = false;
        return 1;
    }
    return 0;
}

long
fit_panel::on_cmd_param_change(FXObject *_txt, FXSelector, void*)
{
    FXTextField *txt = (FXTextField *) _txt;
    param_info* p = (param_info*) txt->getUserData();
    FXString vstr = txt->getText();
    double new_val = strtod(vstr.text(), nullptr);
    unsigned k = this->get_parameter_index(p);
    set_parameter_value(k, new_val);
    if (m_results_target) {
        m_results_target->notify_change();
    }
    if(m_canvas) {
        m_canvas->set_dirty(true);
    }
    return 1;
}

long
fit_panel::on_cmd_plot_autoscale(FXObject*, FXSelector, void*)
{
    m_canvas->update_limits();
    return 1;
}

bool
fit_panel::verify_spectral_range(const char *txt, double ps[])
{
    int nass = sscanf(txt, "%lf-%lf", ps, ps+1);
    return (nass == 2 && ps[1] > ps[0]);
}

bool
fit_panel::update_spectral_range(const char *txt)
{
    double ps[2];
    if(verify_spectral_range(txt, ps)) {
        bool status = set_sampling(ps[0], ps[1]);
        if(status && m_canvas) {
            m_canvas->update_limits();
        }
        return status;
    }
    return false;
}

long
fit_panel::on_change_spectral_range(FXObject *, FXSelector, void*_txt)
{
    const char * txt = (const char *) _txt;

    if(update_spectral_range(txt)) {
        m_wl_entry->setTextColor(regressProApp()->black);
    } else {
        m_wl_entry->setTextColor(regressProApp()->red_warning);
    }

    return 1;
}

long
fit_panel::on_update_spectral_range(FXObject *, FXSelector, void*_txt)
{
    if (range_dirty) {
        config_spectral_range();
        return 1;
    }
    return 0;
}

long
fit_panel::on_cmd_spectral_range(FXObject *, FXSelector, void*)
{
    FXString s = m_wl_entry->getText();
    if(update_spectral_range(s.text())) {
        return 1;
    }
    return 0;
}

long fit_panel::on_cmd_run_fit(FXObject*, FXSelector, void* ptr)
{
    fit_parameters* fps = fit_parameters_new();

    for(unsigned j = 0; j < m_parameters.size(); j++) {
        if(m_parameters[j].selected) {
            fit_parameters_add(fps, &m_parameters[j].fp);
        }
    }

    if(fps->number > 0) {

        run_fit(fps);

        for(unsigned k = 0; k < m_parameters.size(); k++) {
            param_info& p = m_parameters[k];
            FXString s = FXString::value("%g", m_fit->get_parameter_value(k));
            p.text_field->setText(s);
            p.is_dirty = true;
        }

        m_canvas->update_limits();
    }

    fit_parameters_free(fps);

    if (m_results_target) {
        m_results_target->notify_change();
    }

    return 1;
}

void fit_panel::refresh()
{
    for(unsigned k = 0; k < m_parameters.size(); k++) {
        m_parameters[k].is_dirty = true;
    }
    m_canvas->set_dirty(true);
    range_dirty = true;
}

void fit_panel::kill_focus()
{
    for (int i = 0; i < param_matrix->getNumRows(); i++) {
        for (int j = 0; j < param_matrix->getNumColumns(); j++) {
            param_matrix->childAtRowCol(i, j)->killFocus();
        }
    }
}

void fit_panel::set_parameter_value(unsigned k, double val)
{
    double old_value = m_fit->get_parameter_value(k);
    m_fit->set_parameter_value(k, val);
    m_undo_manager.record(new oper_set_parameter(k, old_value, val));
}

bool fit_panel::set_sampling(double s_start, double s_end)
{
    double os, oe;
    m_fit->get_sampling(os, oe);
    bool status = m_fit->set_sampling(s_start, s_end);
    m_undo_manager.record(new oper_set_sampling(os, oe, s_start, s_end));
    return status;
}

double *fit_panel::new_array_fit_values(fit_manager *fm, fit_parameters *fps)
{
    double *values = new(std::nothrow) double[fps->number];
    for (int i = 0; i < fps->number; i++) {
        int k = fm->lookup(&fps->at(i));
        values[i] = (k >= 0 ? m_fit->get_parameter_value(k) : 0.0);
    }
    return values;
}

void fit_panel::run_fit(fit_parameters *fps)
{
    double *old_values = new_array_fit_values(m_fit, fps);
    lmfit_result result = m_fit->run(fps);
    double *new_values = new_array_fit_values(m_fit, fps);
    m_undo_manager.record(new oper_run_fit(fps, old_values, new_values));
    set_fit_result(result);
}

long fit_panel::on_cmd_undo(FXObject*, FXSelector sel, void *)
{
    FXuint id = FXSELID(sel);
    bool done;
    if (id == ID_ACTION_UNDO) {
        done = m_undo_manager.undo(m_fit);
    } else {
        done = m_undo_manager.redo(m_fit);
    }
    if (done) {
        refresh();
        if(m_canvas) {
            m_canvas->update_limits();
        }
        return 1;
    }
    return 0;
}

long fit_panel::on_update_undo_menu(FXObject* sender, FXSelector sel, void *)
{
    FXMenuCommand *menu_cmd = (FXMenuCommand *) sender;
    bool enabled = menu_cmd->isEnabled();
    bool available = (FXSELID(sel) == ID_ACTION_UNDO ? m_undo_manager.has_undo() : m_undo_manager.has_redo());
    if (enabled && !available) {
        menu_cmd->disable();
        return 1;
    }
    if (!enabled && available) {
        menu_cmd->enable();
        return 1;
    }
    return 0;
}

long fit_panel::on_cmd_plot_copy(FXObject *, FXSelector, void *)
{
    m_canvas->acquire_clipboard_as_text();
    return 1;
}

long fit_panel::on_cmd_plot_copy_as_image(FXObject *, FXSelector, void *)
{
    m_canvas->acquire_clipboard_as_image();
    return 1;
}
