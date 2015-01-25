#include <fxkeys.h>
#include <stdint.h>

#include "multifit_window.h"
#include "fit_params_utils.h"
#include "str.h"
#include "stack.h"

// Map
FXDEFMAP(multifit_window) multifit_window_map[]= {
    FXMAPFUNC(SEL_COMMAND, multifit_window::ID_ADD_FIT, multifit_window::on_cmd_add_fit),
    FXMAPFUNC(SEL_COMMAND, multifit_window::ID_ADD_CONSTR, multifit_window::on_cmd_add_constr),
    FXMAPFUNC(SEL_SELECTED, multifit_window::ID_PARAM_INDIV, multifit_window::on_select_param),
    FXMAPFUNC(SEL_SELECTED, multifit_window::ID_PARAM_CONSTR, multifit_window::on_select_param),
    FXMAPFUNC(SEL_COMMAND, multifit_window::ID_STACK_CHANGE, multifit_window::on_cmd_stack_change),
    FXMAPFUNC(SEL_COMMAND, multifit_window::ID_STACK_SHIFT, multifit_window::on_cmd_stack_shift),
    FXMAPFUNC(SEL_KEYPRESS, multifit_window::ID_PARAM_INDIV, multifit_window::on_keypress_parameter),
    FXMAPFUNC(SEL_KEYPRESS, multifit_window::ID_PARAM_CONSTR, multifit_window::on_keypress_parameter),
};

FXIMPLEMENT(multifit_window,FXDialogBox,multifit_window_map,ARRAYNUMBER(multifit_window_map));

multifit_window::multifit_window(fit_recipe *rcp, FXWindow* topwin, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(topwin, "Multi-Sample Recipe Edit", opts, 0, 0, 380, 320, pl, pr, pt, pb, hs, vs),
    recipe(rcp), param_list(NULL)
{
    FXVerticalFrame *vf = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXHorizontalFrame *hf = new FXHorizontalFrame(vf, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXGroupBox *ipgroup = new FXGroupBox(hf, "Individual Parameters", GROUPBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_LINE);
    indiv_listbox = new FXList(ipgroup, this, ID_PARAM_INDIV, LIST_SINGLESELECT|LAYOUT_FILL_Y|LAYOUT_FILL_X);
    FXGroupBox *csgroup = new FXGroupBox(hf, "Constraints", GROUPBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_LINE);
    constr_listbox = new FXList(csgroup, this, ID_PARAM_CONSTR, LIST_SINGLESELECT|LAYOUT_FILL_Y|LAYOUT_FILL_X);

    iparameters = fit_parameters_new();
    cparameters = fit_parameters_new();

    new FXHorizontalSeparator(vf, SEPARATOR_GROOVE|LAYOUT_FILL_X);

    FXHorizontalFrame *bhf = new FXHorizontalFrame(vf, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXGroupBox *plgroup = new FXGroupBox(bhf, "Fit Parameters", GROUPBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_LINE);
    param_listbox = new FXListBox(plgroup, this, ID_PARAM_SELECT);
    param_listbox->setNumVisible(8);

    FXVerticalFrame *btframe = new FXVerticalFrame(bhf, LAYOUT_FILL_Y);
    new FXButton(btframe, "Add to fit parameters", NULL, this, ID_ADD_FIT);
    new FXButton(btframe, "Add to constraints", NULL, this, ID_ADD_CONSTR);

    setup_parameters_list();
}

multifit_window::~multifit_window() 
{
    if (param_list) {
        fit_parameters_free(param_list);
    }
    fit_parameters_free(iparameters);
    fit_parameters_free(cparameters);
}

void multifit_window::setup_parameters_list()
{
    if (param_list) {
        fit_parameters_free(param_list);
    }
    param_list = listbox_populate_all_parameters(param_listbox, recipe->stack);
}

const fit_param_t *multifit_window::selected_parameter() const
{
    FXint no = param_listbox->getCurrentItem();
    int index = (intptr_t)(param_listbox->getItemData(no)) - 1;
    return index >= 0 ? &param_list->values[index] : NULL;
}

long multifit_window::on_cmd_add_fit(FXObject *, FXSelector, void *)
{
    const fit_param_t *fp = selected_parameter();
    int i = fit_parameters_find(iparameters, fp);
    if (i < 0) {
        fit_parameters_add(iparameters, fp);
        indiv_listbox->appendItem(format_fit_parameter(fp));
    }
    return 1;
}

long multifit_window::on_cmd_add_constr(FXObject *, FXSelector, void *)
{
    const fit_param_t *fp = selected_parameter();
    int i = fit_parameters_find(cparameters, fp);
    if (i < 0) {
        fit_parameters_add(cparameters, fp);
        constr_listbox->appendItem(format_fit_parameter(fp));
    }
    return 1;
}

long multifit_window::on_select_param(FXObject *, FXSelector sel, void *)
{
    int id = FXSELID(sel);
    const fit_param_t *selfp;
    if (id == ID_PARAM_INDIV) {
        FXint index = indiv_listbox->getCurrentItem();
        /* Get the selected fit parameter. */
        selfp = &iparameters->values[index];
    } else {
        FXint index = constr_listbox->getCurrentItem();
        /* Get the selected fit parameter. */
        selfp = &cparameters->values[index];
    }
    /* Find the index of the fit parameter in the list of all
       possible parameters. */
    int fp_index = fit_parameters_find(param_list, selfp);
    /* Select in the listbox the given parameter. */
    listbox_select_parameter(param_listbox, fp_index);
    return 1;
}

long multifit_window::on_cmd_stack_change(FXObject *, FXSelector, void *)
{
    list_populate(indiv_listbox, iparameters, NULL, true);
    list_populate(constr_listbox, cparameters, NULL, true);
    setup_parameters_list();
    return 1;
}

long multifit_window::on_cmd_stack_shift(FXObject *, FXSelector, void *ptr)
{
    const shift_info *shift = (const shift_info *) ptr;
    fit_parameters_fix_layer_shift(iparameters, *shift);
    fit_parameters_fix_layer_shift(cparameters, *shift);
    return 1;
}

long multifit_window::on_keypress_parameter(FXObject *, FXSelector sel, void *ptr)
{
    int id = FXSELID(sel);
    FXEvent* event=(FXEvent*)ptr;
    FXList *list = (id == ID_PARAM_INDIV ? indiv_listbox : constr_listbox);
    fit_parameters *fps = (id == ID_PARAM_INDIV ? iparameters : cparameters);
    switch(event->code) {
    case KEY_Delete:
    {
        FXint index = list->getCurrentItem();
        if (index >= 0) {
            list->removeItem(index);
            fit_parameters_remove(fps, index);
            return 1;
        }
        break;
    }
    default:
        /* */ ;
    }
    return 0;
}
