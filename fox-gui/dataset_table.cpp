#include "dataset_table.h"
#include "fit_params_utils.h"

// Map
FXDEFMAP(dataset_table) dataset_table_map[]= {
    FXMAPFUNC(SEL_COMMAND, dataset_table::ID_SELECT_COLUMN_INDEX, dataset_table::on_cmd_select_column),
    FXMAPFUNC(SEL_COMMAND, dataset_table::ID_STACK_CHANGE, dataset_table::on_cmd_stack_change),
    FXMAPFUNCS(SEL_COMMAND, dataset_table::ID_FIT_PARAM, dataset_table::ID_FIT_PARAM_LAST, dataset_table::on_cmd_fit_param),
};

FXIMPLEMENT(dataset_table,FXTable,dataset_table_map,ARRAYNUMBER(dataset_table_map));

dataset_table::dataset_table(fit_recipe *recipe, FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb)
    : FXTable(p, tgt, sel, opts, x, y, w, h, pl, pr, pt, pb), entries_no(0)
{
    fit_params = fit_parameters_new();
    stack_get_all_parameters(recipe->stack, fit_params);
    popupmenu = fit_parameters_menu(this, this, ID_FIT_PARAM, fit_params);
}

dataset_table::~dataset_table()
{
    delete popupmenu;
}

void dataset_table::stack_change_update(stack_t *stack)
{
    fit_parameters_clear(fit_params);
    stack_get_all_parameters(stack, fit_params);
    delete popupmenu;
    popupmenu = fit_parameters_menu(this, this, ID_FIT_PARAM, fit_params);
    popupmenu->create();
}

long dataset_table::on_cmd_stack_change(FXObject *, FXSelector, void *data)
{
    stack_change_update((stack_t *)data);
    return 1;
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

void dataset_table::link_parameter(const fit_param_t *fp, const int column)
{
    for (fit_param_node *p = fplink.top; p; p = p->next) {
        if (p->column == column) {
            fplink.params->values[p->fp_index] = *fp;
            return;
        }
    }
    int fp_index = fit_parameters_add(fplink.params, fp);
    fit_param_node *new_node = new fit_param_node(fp_index, column);
    new_node->next = fplink.top;
    fplink.top = new_node;
}

long dataset_table::on_cmd_fit_param(FXObject *obj, FXSelector sel, void *ptr)
{
    int fp_index = FXSELID(sel) - ID_FIT_PARAM;
    const fit_param_t *fp = fit_params->values + fp_index;
    link_parameter(fp, popup_col);
    str_t name;
    str_init(name, 16);
    get_full_param_name(fit_params->values + fp_index, name);
    setColumnText(popup_col, CSTR(name));
    str_free(name);
    return 1;
}

bool dataset_table::get_spectra_list(spectrum *spectra_list[], FXString& error_filename)
{
    for (int i = 0; i < entries_no; i++) {
        FXString name = getItemText(i, 0);
        spectrum *s = load_gener_spectrum(name.text());
        if (!s) {
            error_filename = name;
            return false;
        }
        spectra_list[i] = s;
    }
    return true;
}

bool dataset_table::get_values(int row, const fit_parameters *fps, double value_array[], int& error_col)
{
    FXString cell_text;
    for (unsigned i = 0; i < fps->number; i++) {
        const fit_param_t *fp = &fps->values[i];
        const char *ctext = NULL;
        for (fit_param_node *p = fplink.top; p; p = p->next) {
            const fit_param_t *xfp = &fplink.params->values[p->fp_index];
            if (fit_param_compare(xfp, fp) == 0) {
                cell_text = getItemText(row, p->column);
                ctext = cell_text.text();
                char *endptr;
                value_array[i] = strtod(ctext, &endptr);
                if (endptr == ctext) {
                    error_col = p->column;
                    return false;
                }
                break;
            }
        }
        if (ctext == NULL) { /* Fit parameter not found. */
            error_col = -1;
            return false;
        }
    }
    return true;
}
