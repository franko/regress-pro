#include "dispers_ui_utils.h"
#include "dispers_edit_window.h"
#include "dispers_chooser.h"

disp_t *ui_edit_dispersion(FXWindow *win, disp_t *disp)
{
    disp_t *edit_disp = disp_copy(disp);
    dispers_edit_window *edit_win = new dispers_edit_window(edit_disp, win, DECOR_TITLE|DECOR_BORDER, 0, 0, 400, 320);
    if (edit_win->execute() == TRUE) {
        disp_free(disp);
        return edit_disp;
    }
    disp_free(edit_disp);
    return NULL;
}

disp_t *ui_choose_dispersion(FXApp *app)
{
    dispers_chooser chooser(app);
    if (chooser.execute() != TRUE) return NULL;
    return chooser.get_dispersion();
}
