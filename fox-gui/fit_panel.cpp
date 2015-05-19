#include "fit_panel.h"

#include "fx_numeric_field.h"
#include "Strcpp.h"
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
};

// Object implementation
FXIMPLEMENT(fit_panel,FXHorizontalFrame,fit_panel_map,ARRAYNUMBER(fit_panel_map));

fit_panel::fit_panel(fit_manager* fit, FXComposite *p, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXHorizontalFrame(p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs),
      m_canvas(0), m_fit(fit), m_results_target(NULL)
{
    scroll_window = new FXScrollWindow(this, VSCROLLER_ALWAYS | HSCROLLING_OFF | LAYOUT_FILL_Y);
    setup();
}

void fit_panel::clear()
{
    delete param_matrix;
    delete m_canvas;
}

void fit_panel::setup()
{
    param_matrix = new FXMatrix(scroll_window, 2, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_Y|MATRIX_BY_COLUMNS, 0, 0, 0, 0, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, 1, 1);

    new FXLabel(param_matrix, "Range");
    m_wl_entry = new FXTextField(param_matrix, 10, this, ID_SPECTR_RANGE, FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_ROW);

    double wls, wle, wld;
    m_fit->get_sampling(wls, wle, wld);

    if(wld == 0.0) {
        m_wl_entry->setText(FXStringFormat("%.3g-%.3g", wls, wle));
    } else {
        m_wl_entry->setText(FXStringFormat("%.3g-%.3g,%g", wls, wle, wld));
    }

    m_parameters.resize(m_fit->parameters_number());

    Str pname;
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

        get_param_name(&p->fp, pname.str());
        FXString fxpname((const FXchar *) pname.cstr());
        FXCheckButton* bt = new FXCheckButton(param_matrix, fxpname, this, ID_PARAM_SELECT);
        FXTextField* tf = new fx_numeric_field(param_matrix, 10, this, ID_PARAM_VALUE, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);

        tf->setUserData(p);
        bt->setUserData(p);

        p->text_field = tf;
        p->selected = false;
        p->is_dirty = true;
    }

    m_canvas = new plot_canvas(this, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_fit->config_plot(m_canvas);
}

void fit_panel::reload()
{
    clear();
    setup();
    param_matrix->create();
    m_canvas->create();
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
        FXString s = FXStringFormat("%g", m_fit->get_parameter_value(k));
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
    double new_val = strtod(vstr.text(), NULL);
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
    int nass = sscanf(txt, "%lf-%lf,%lf", ps, ps+1, ps+2);

    if(nass < 2) {
        return false;
    }

    if(nass == 2) {
        ps[2] = 0.0;
    } else {
        if(ps[2] < 0.1) {
            return false;
        }

        if(int((ps[1]-ps[0])/ps[2]) < 2) {
            return false;
        }
    }

    return (ps[0] >= 50.0 && ps[1] <= 5000.0 && ps[1] > ps[0]);
}

bool
fit_panel::update_spectral_range(const char *txt)
{
    double ps[3];

    if(verify_spectral_range(txt, ps)) {
        return set_sampling(ps[0], ps[1], ps[2]);
    }

    return false;
}

long
fit_panel::on_change_spectral_range(FXObject *, FXSelector, void*_txt)
{
    const char * txt = (const char *) _txt;

    if(update_spectral_range(txt)) {
        m_wl_entry->setTextColor(regressProApp()->black);
        if(m_canvas) {
            m_canvas->update_limits();
        }
    } else {
        m_wl_entry->setTextColor(regressProApp()->red_warning);
    }

    return 1;
}

long
fit_panel::on_cmd_spectral_range(FXObject *, FXSelector, void*)
{
    FXString s = m_wl_entry->getText();
    if(update_spectral_range(s.text())) {
        if(m_canvas) {
            m_canvas->update_limits();
        }
        return 1;
    }
    return 0;
}

long fit_panel::on_cmd_run_fit(FXObject*, FXSelector, void* ptr)
{
    struct fit_parameters* fps = fit_parameters_new();

    for(unsigned j = 0; j < m_parameters.size(); j++) {
        if(m_parameters[j].selected) {
            fit_parameters_add(fps, &m_parameters[j].fp);
        }
    }

    if(fps->number > 0) {

        run_fit(fps);

        for(unsigned k = 0; k < m_parameters.size(); k++) {
            param_info& p = m_parameters[k];
            FXString s = FXStringFormat("%g", m_fit->get_parameter_value(k));
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
    m_fit->set_parameter_value(k, val);
}

bool fit_panel::set_sampling(double s_start, double s_end, double s_step)
{
    return m_fit->set_sampling(s_start, s_end, s_step);
}

void fit_panel::run_fit(fit_parameters *fps)
{
    m_fit->run(fps);
}
