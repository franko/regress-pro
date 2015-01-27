#include "dataset_table.h"
#include "fit_params_utils.h"

// Map
FXDEFMAP(dataset_table) dataset_table_map[]= {
    // FXMAPFUNC(SEL_LEFTBUTTONPRESS, 0, dataset_table::on_left_button),
    FXMAPFUNC(SEL_COMMAND, dataset_table::ID_SELECT_COLUMN_INDEX, dataset_table::on_cmd_select_column),
};

FXIMPLEMENT(dataset_table,FXTable,dataset_table_map,ARRAYNUMBER(dataset_table_map));

dataset_table::dataset_table(fit_recipe *rcp, FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb)
    : FXTable(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb), entries_no(0),
    recipe(rcp)
{
    fit_params = fit_parameters_new();
    stack_get_all_parameters(recipe->stack, fit_params);
    popupmenu = fit_parameters_menu(this, this, ID_FIT_PARAM, fit_params);
}

dataset_table::~dataset_table()
{
    delete popupmenu;
}

void dataset_table::create()
{
    FXTable::create();
    popupmenu->create();
}

void dataset_table::append_filenames(FXString *filenames)
{
    int count = 0;
    for(FXString *p = filenames; *p != ""; p++, count++) ;
    if (entries_no + count > getNumRows()) {
        int del = entries_no + count - getNumRows();
        insertRows(entries_no, del);
    }
    char rowlabel[64];
    for (int i = 0; filenames[i] != ""; i++) {
        int n = entries_no + i;
        sprintf(rowlabel, "%d", n + 1);
        setRowText(n, rowlabel);
        setItemText(n, 0, filenames[i]);
    }
    entries_no += count;
}

long dataset_table::on_left_button(FXObject *obj, FXSelector sel, void *ptr)
{
    FXEvent *event = (FXEvent *)ptr;
    int row= rowAtY(event->win_y);
    int col= colAtX(event->win_x);
    fprintf(stderr, ">> %d %d\n", row, col);
    return FXTable::onLeftBtnPress(obj, sel, ptr);
}

long dataset_table::on_cmd_select_column(FXObject *obj, FXSelector sel, void *ptr)
{
    FXint col = (FXint)(FXival)ptr;
    popupmenu->popup(NULL, 50, 50);
    getApp()->runModalWhileShown(popupmenu);
    return 1;
}
