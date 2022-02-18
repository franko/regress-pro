#include <stdint.h>
#include "fit_params_utils.h"
#include "fit-params.h"
#include "acquisition.h"

fit_parameters *listbox_populate_all_parameters(FXListBox *listbox, stack_t *stack, const struct acquisition_parameters *acquisition)
{
    fit_parameters *fps = fit_parameters_new();
    stack_get_all_parameters(stack, fps);
    acquisition_get_all_parameters(acquisition, fps);

    listbox->clearItems();

    str_t name;
    str_init(name, 16);

    int current_layer = 0;
    for (int i = 0; i < int(fps->number); i++) {
        const fit_param_t *fp = &fps->at(i);
        if (fp->id >= PID_ACQUISITION_PARAMETER && current_layer >= 0) {
            listbox->appendItem("-- acquisition");
            current_layer = -1;
        }
        if (fp->id == PID_LAYER_N && fp->layer_nb != current_layer) {
            str_printf(name, "-- layer %d", fp->layer_nb);
            listbox->appendItem(CSTR(name));
            current_layer = fp->layer_nb;
        }
        get_full_param_name(fp, name);
        listbox->appendItem(CSTR(name), nullptr, (void*) (intptr_t) (i + 1));
    }

    str_free(name);
    return fps;
}

FXString
format_fit_parameter(const fit_param_t *fp, const seed_t *value)
{
    FXString txt;
    str_t name;
    str_init(name, 16);
    get_full_param_name(fp, name);
    if (value && value->type == SEED_SIMPLE) {
        txt.format("%s: %g", CSTR(name), value->seed);
    } else if (value && value->type == SEED_RANGE) {
        txt.format("%s: %g +/- %g", CSTR(name), value->seed, value->delta);
    } else if (value) {
        txt.format("%s: ?", CSTR(name));
    } else {
        txt.format("%s", CSTR(name));
    }
    str_free(name);
    return txt;
}

int listbox_select_parameter(FXListBox *listbox, int fp_index)
{
    for (int j = 0; j < listbox->getNumItems(); j++) {
        if ((intptr_t)(listbox->getItemData(j)) - 1 == fp_index) {
            listbox->setCurrentItem(j, true);
            return 1;
        }
    }
    return 0;
}

void list_populate(FXList *list, fit_parameters *fps, seeds_list *seeds, bool clear)
{
    if (clear) {
        list->clearItems();
    }
    for (int i = 0; i < fps->number; i++) {
        const fit_param_t *fp = &fps->at(i);
        const seed_t *value = (seeds ? &seeds->at(i) : nullptr);
        list->appendItem(format_fit_parameter(fp, value));
    }
}

FXMenuPane *fit_parameters_menu(FXWindow *win, FXObject *target, FXSelector sel, fit_parameters *fps)
{
    FXMenuPane *topmenu = new FXMenuPane(win);
    FXMenuPane *menu = topmenu;

    str_t name;
    str_init(name, 16);
    int current_layer = 0;
    for (int i = 0; i < fps->number; i++) {
        const fit_param_t *fp = &fps->at(i);
        if (fp->id == PID_LAYER_N && fp->layer_nb != current_layer) {
            menu = new FXMenuPane(win);
            str_printf(name, "Layer %d", fp->layer_nb);
            new FXMenuCascade(topmenu, CSTR(name), nullptr, menu);
            current_layer = fp->layer_nb;
        } else if (fp->id != PID_LAYER_N) {
            menu = topmenu;
        }
        get_full_param_name(fp, name);
        new FXMenuCommand(menu, CSTR(name), nullptr, target, sel + i);
    }
    str_free(name);
    return topmenu;
}
