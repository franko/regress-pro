#include "dataset_window.h"

// Map
FXDEFMAP(dataset_window) dataset_window_map[]= {
    FXMAPFUNC(SEL_COMMAND, dataset_window::ID_DELETE, dataset_window::onCmdHide),
};

FXIMPLEMENT(dataset_window,FXDialogBox,dataset_window_map,ARRAYNUMBER(dataset_window_map));

dataset_window::dataset_window(fit_recipe *rcp, FXWindow *topwin, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(topwin, "Spectra Dataset", opts, 0, 0, 540, 400, pl, pr, pt, pb, hs, vs)
{
    FXHorizontalFrame *hframe = new FXHorizontalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    table = new dataset_table(rcp, hframe, this, ID_TABLE, TABLE_COL_SIZABLE|TABLE_ROW_SIZABLE|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    FXVerticalFrame *bframe = new FXVerticalFrame(hframe, LAYOUT_FILL_Y);
    new FXButton(bframe, "Add Files", nullptr, table, dataset_table::ID_ADD_FILES);
    new FXButton(bframe, "Remove all", nullptr, table, dataset_table::ID_REMOVE_FILES);
}
