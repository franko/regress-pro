#include "dataset_window.h"

// Map
FXDEFMAP(dataset_window) dataset_window_map[]= {
};

FXIMPLEMENT(dataset_window,FXDialogBox,dataset_window_map,ARRAYNUMBER(dataset_window_map));

dataset_window::dataset_window(fit_recipe *rcp, FXWindow *topwin, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(topwin, "Spectra Dataset", opts, 0, 0, 540, 400, pl, pr, pt, pb, hs, vs)
{
    FXHorizontalFrame *hframe = new FXHorizontalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    table = new dataset_table(rcp, hframe, this, ID_TABLE, TABLE_COL_SIZABLE|TABLE_ROW_SIZABLE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    table->setTableSize(20, 6);
    table->setRowHeaderWidth(28);
    table->setColumnText(0, "Filename");

    FXVerticalFrame *bframe = new FXVerticalFrame(hframe, LAYOUT_FILL_Y);
    new FXButton(bframe, "Add Files", NULL, table, dataset_table::ID_ADD_FILES);
    new FXButton(bframe, "Remove all", NULL, table, dataset_table::ID_REMOVE_FILES);
}
