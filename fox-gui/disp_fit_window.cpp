#include "disp_fit_window.h"
#include "disp_fit_manager.h"
#include "dispers_chooser.h"

// Map
FXDEFMAP(disp_fit_window) disp_fit_window_map[]= {
    FXMAPFUNC(SEL_COMMAND, disp_fit_window::ID_SELECT_REF, disp_fit_window::on_cmd_select),
    FXMAPFUNC(SEL_COMMAND, disp_fit_window::ID_SELECT_MODEL, disp_fit_window::on_cmd_select),
};

// Object implementation
FXIMPLEMENT(disp_fit_window,FXMainWindow,disp_fit_window_map,ARRAYNUMBER(disp_fit_window_map));

disp_fit_window::disp_fit_window(disp_fit_manager* fit, FXApp* a, const FXString& name,FXIcon *ic,FXIcon *mi,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
    FXMainWindow(a, name, ic, mi, opts, x, y, w, h, pl, pr, pt, pb, hs, vs),
    m_fit_manager(fit)
{
    menubar = new FXMenuBar(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
    statusbar = new FXStatusBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|FRAME_RAISED|STATUSBAR_WITH_DRAGCORNER);

    m_fit_panel = new fit_panel(fit, this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    fitmenu = new FXMenuPane(this);
    new FXMenuCommand(fitmenu, "&Run", NULL, m_fit_panel, fit_panel::ID_RUN_FIT);
    new FXMenuTitle(menubar, "&Fit", NULL, fitmenu);

    plotmenu = new FXMenuPane(this);
    new FXMenuCommand(plotmenu, "&Auto Scale", NULL, m_fit_panel, fit_panel::ID_PLOT_SCALE);
    new FXMenuTitle(menubar, "&Plot", NULL, plotmenu);

    dispmenu = new FXMenuPane(this);
    new FXMenuCommand(dispmenu, "Select Reference", NULL, this, ID_SELECT_REF);
    new FXMenuCommand(dispmenu, "Select Model", NULL, this, ID_SELECT_MODEL);
    new FXMenuTitle(menubar, "&Dispersion", NULL, dispmenu);
}

disp_fit_window::~disp_fit_window()
{
    delete fitmenu;
    delete plotmenu;
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
