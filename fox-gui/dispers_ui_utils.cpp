#include "dispers_ui_utils.h"
#include "dispers_edit_window.h"
#include "dispers_chooser.h"

disp_t *ui_edit_dispersion(FXWindow *win, disp_t *disp)
{
    disp_t *edit_disp = disp_copy(disp);
    dispers_edit_window *edit_win = new dispers_edit_window(edit_disp, win, DECOR_TITLE|DECOR_BORDER, 0, 0, 400, 360);
    if (edit_win->execute()) {
        disp_free(disp);
        return edit_disp;
    }
    disp_free(edit_disp);
    return NULL;
}

disp_t *ui_choose_dispersion(FXWindow *win)
{
    dispers_chooser chooser(win);
    if (!chooser.execute()) return NULL;
    return chooser.get_dispersion();
}
