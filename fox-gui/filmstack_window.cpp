#include "filmstack_window.h"

// Map
FXDEFMAP(filmstack_window) filmstack_window_map[],  filmstack_window::ID_PARAM_SELECT, filmstack_window::on_cmd_param_select),
    // FXMAPFUNC(SEL_COMMAND, filmstack_window::ID_PARAM_VALUE,  filmstack_window::on_cmd_param_change),
    // FXMAPFUNC(SEL_CHANGED, filmstack_window::ID_PARAM_VALUE,  filmstack_window::on_cmd_param_change),
    // FXMAPFUNC(SEL_UPDATE,  filmstack_window::ID_PARAM_VALUE,  filmstack_window::on_update_param),
    // FXMAPFUNC(SEL_COMMAND, filmstack_window::ID_RUN_FIT,      filmstack_window::on_cmd_run_fit),
    // FXMAPFUNC(SEL_COMMAND, filmstack_window::ID_PLOT_SCALE,   filmstack_window::on_cmd_plot_autoscale),
    // FXMAPFUNC(SEL_COMMAND, filmstack_window::ID_SPECTR_RANGE, filmstack_window::on_cmd_spectral_range),
    // FXMAPFUNC(SEL_CHANGED, filmstack_window::ID_SPECTR_RANGE, filmstack_window::on_change_spectral_range),
};

// Object implementation
FXIMPLEMENT(filmstack_window,FXDialogBox,filmstack_window_map,ARRAYNUMBER(filmstack_window_map));

filmstack_window::filmstack_window(FXApp* a, FXIcon *ic, FXIcon *mi, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(a, "Film Stack", opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{

}
