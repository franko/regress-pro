#include "disp_fit_window.h"
#include "disp_fit_manager.h"
#include "dispers_ui_utils.h"
#include "dispers_edit_window.h"
#include "dispers-library.h"

// Map
FXDEFMAP(disp_fit_window) disp_fit_window_map[]= {
    FXMAPFUNC(SEL_COMMAND, disp_fit_window::ID_SELECT_REF, disp_fit_window::on_cmd_select),
    FXMAPFUNC(SEL_COMMAND, disp_fit_window::ID_SELECT_MODEL, disp_fit_window::on_cmd_select),
    FXMAPFUNC(SEL_COMMAND, disp_fit_window::ID_EDIT_MODEL, disp_fit_window::on_cmd_edit_model),
    FXMAPFUNC(SEL_COMMAND, disp_fit_window::ID_SAVE_USERLIB, disp_fit_window::on_cmd_save_userlib),
    FXMAPFUNC(SEL_COMMAND, disp_fit_window::ID_DELETE, disp_fit_window::onCmdHide),
};

// Object implementation
FXIMPLEMENT(disp_fit_window,fit_window,disp_fit_window_map,ARRAYNUMBER(disp_fit_window_map));

disp_fit_window::disp_fit_window(disp_fit_manager* fit, FXWindow *win, const FXString& name,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
    fit_window(fit, win, name, opts, x, y, w, h, pl, pr, pt, pb, hs, vs),
    m_fit_manager(fit)
{
    dispmenu = new FXMenuPane(this);
    new FXMenuCommand(dispmenu, "Select Reference", nullptr, this, ID_SELECT_REF);
    new FXMenuCommand(dispmenu, "Select Model", nullptr, this, ID_SELECT_MODEL);
    new FXMenuCommand(dispmenu, "Edit Model", nullptr, this, ID_EDIT_MODEL);
    new FXMenuCommand(dispmenu, "Save Model to User Library", nullptr, this, ID_SAVE_USERLIB);
    new FXMenuTitle(menubar, "&Dispersion", nullptr, dispmenu);
}

disp_fit_window::~disp_fit_window()
{
    delete dispmenu;
    delete m_fit_manager;
}

long disp_fit_window::on_cmd_select(FXObject *, FXSelector sel, void *)
{
    FXuint id = FXSELID(sel);
    disp_t *d = ui_choose_dispersion(this);
    if (!d) return 1;
    if (id == ID_SELECT_REF) {
        m_fit_manager->set_reference(d);
    } else {
        m_fit_manager->set_model(d);
    }
    m_fit_panel->reload();
    return 1;
}

long disp_fit_window::on_cmd_save_userlib(FXObject *, FXSelector, void *)
{
    disp_t *disp = disp_copy(m_fit_manager->model_ref());
    dispers_edit_window edit_win(disp, this, DECOR_TITLE|DECOR_BORDER, 0, 0, 400, 320);
    if (edit_win.execute() == TRUE) {
        disp_list_add(user_lib, disp, nullptr);
    }
    return 1;
}

long disp_fit_window::on_cmd_edit_model(FXObject *, FXSelector, void *)
{
    disp_t *disp = m_fit_manager->model_ref();
    disp_t *new_disp = ui_edit_dispersion(this, disp);
    if (new_disp) {
        m_fit_manager->set_model_ref(new_disp);
        m_fit_panel->reload();
    }
    return 1;
}
