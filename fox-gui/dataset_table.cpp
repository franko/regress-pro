#include "dataset_table.h"
#include "fit_params_utils.h"

// Map
FXDEFMAP(dataset_table) dataset_table_map[]= {
    FXMAPFUNC(SEL_COMMAND, dataset_table::ID_SELECT_COLUMN_INDEX, dataset_table::on_cmd_select_column),
    FXMAPFUNCS(SEL_COMMAND, dataset_table::ID_FIT_PARAM, dataset_table::ID_FIT_PARAM_LAST, dataset_table::on_cmd_fit_param),
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

long dataset_table::on_cmd_select_column(FXObject *obj, FXSelector sel, void *ptr)
{
    popup_col = (FXint)(FXival)ptr;
    if (popup_col == 0) return 0;
    FXint x = getShell()->getX() + colHeader->getX() + colHeader->getItemOffset(popup_col);
    FXint y = getShell()->getY() + colHeader->getY();
    popupmenu->popup(NULL, x, y);
    getApp()->runModalWhileShown(popupmenu);
    return 1;
}

long dataset_table::on_cmd_fit_param(FXObject *obj, FXSelector sel, void *ptr)
{
    int fpindex = FXSELID(sel) - ID_FIT_PARAM;
    const fit_param_t *fp = fit_params->values + fpindex;
    str_t name;
    str_init(name, 16);
    get_full_param_name(fp, name);
    setColumnText(popup_col, CSTR(name));
    str_free(name);
    return 1;
}
