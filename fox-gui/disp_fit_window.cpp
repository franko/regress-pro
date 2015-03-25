#include "disp_fit_window.h"
#include "disp_fit_manager.h"
#include "dispers_chooser.h"
#include "dispers-library.h"

// Map
FXDEFMAP(disp_fit_window) disp_fit_window_map[]= {
    FXMAPFUNC(SEL_COMMAND, disp_fit_window::ID_SELECT_REF, disp_fit_window::on_cmd_select),
    FXMAPFUNC(SEL_COMMAND, disp_fit_window::ID_SELECT_MODEL, disp_fit_window::on_cmd_select),
    FXMAPFUNC(SEL_COMMAND, disp_fit_window::ID_SAVE_USERLIB, disp_fit_window::on_cmd_save_userlib)
};

// Object implementation
FXIMPLEMENT(disp_fit_window,fit_window,disp_fit_window_map,ARRAYNUMBER(disp_fit_window_map));

disp_fit_window::disp_fit_window(disp_fit_manager* fit, FXApp* a, const FXString& name,FXIcon *ic,FXIcon *mi,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
    fit_window(fit, a, name, ic, mi, opts, x, y, w, h, pl, pr, pt, pb, hs, vs),
    m_fit_manager(fit)
{
    dispmenu = new FXMenuPane(this);
    new FXMenuCommand(dispmenu, "Select Reference", NULL, this, ID_SELECT_REF);
    new FXMenuCommand(dispmenu, "Select Model", NULL, this, ID_SELECT_MODEL);
    new FXMenuCommand(dispmenu, "Save Model to User Library", NULL, this, ID_SAVE_USERLIB);
    new FXMenuTitle(menubar, "&Dispersion", NULL, dispmenu);
}

disp_fit_window::~disp_fit_window()
{
    delete dispmenu;
}

long disp_fit_window::on_cmd_select(FXObject *, FXSelector sel, void *)
{
    FXuint id = FXSELID(sel);
    dispers_chooser chooser(this->getApp());
    disp_t *d;
    if (chooser.execute() == TRUE) {
        d = chooser.get_dispersion();
        if (!d) return 1;
        if (id == ID_SELECT_REF) {
            m_fit_manager->set_reference(d);
        } else {
            m_fit_manager->set_model(d);
        }
        m_fit_panel->reload();
    }
    return 1;
}

long disp_fit_window::on_cmd_save_userlib(FXObject *, FXSelector, void *)
{
    disp_list_add(user_lib, m_fit_manager->get_model());
    return 1;
}
