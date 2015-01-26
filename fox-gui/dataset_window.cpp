#include "dataset_window.h"

// Map
FXDEFMAP(dataset_window) dataset_window_map[]= {
    FXMAPFUNC(SEL_COMMAND, dataset_window::ID_ADD_FILES, dataset_window::on_cmd_add_files),
    FXMAPFUNC(SEL_COMMAND, dataset_window::ID_REMOVE_FILES, dataset_window::on_cmd_remove_files),
};

FXIMPLEMENT(dataset_window,FXDialogBox,dataset_window_map,ARRAYNUMBER(dataset_window_map));

dataset_window::dataset_window(FXWindow *topwin, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(topwin, "Spectra Dataset", opts, 0, 0, 540, 400, pl, pr, pt, pb, hs, vs)
{
    FXHorizontalFrame *hframe = new FXHorizontalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    table = new dataset_table(hframe, this, ID_TABLE, TABLE_COL_SIZABLE|TABLE_ROW_SIZABLE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    table->setTableSize(20, 6);
    table->setRowHeaderWidth(28);
    table->setColumnText(0, "Filename");

    FXVerticalFrame *bframe = new FXVerticalFrame(hframe, LAYOUT_FILL_Y);
    new FXButton(bframe, "Add Files", NULL, this, ID_ADD_FILES);
    new FXButton(bframe, "Remove all", NULL, this, ID_REMOVE_FILES);
}

long dataset_window::on_cmd_add_files(FXObject *, FXSelector, void *)
{
    FXFileDialog open(this,"Open Spectra");
    open.setSelectMode(SELECTFILE_MULTIPLE_ALL);
    open.setPatternList("Spectra File (*.dat)\nAny Files (*)");
    if (open.execute()) {
        FXString *filenames = open.getFilenames();
        if (filenames) {
            table->append_filenames(filenames);
        }
    }
    return 1;
}

long dataset_window::on_cmd_remove_files(FXObject *, FXSelector, void *)
{
    return 0;
}
