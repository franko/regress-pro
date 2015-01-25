#include <stdint.h>
#include "fit_params_utils.h"
#include "fit-params.h"

fit_parameters *listbox_populate_all_parameters(FXListBox *listbox, stack_t *stack)
{
    fit_parameters *fps = fit_parameters_new();
    stack_get_all_parameters(stack, fps);

    listbox->clearItems();

    str_t name;
    str_init(name, 16);
    int current_layer = 0;
    for (size_t i = 0; i < fps->number; i++) {
        fit_param_t *fp = &fps->values[i];
        if (fp->id == PID_LAYER_N && fp->layer_nb != current_layer) {
            str_printf(name, "-- layer %d", fp->layer_nb);
            listbox->appendItem(CSTR(name));
            current_layer = fp->layer_nb;
        }
        get_full_param_name(fp, name);
        listbox->appendItem(CSTR(name), NULL, (void*) (i + 1));
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
        txt.format("%s, %g", CSTR(name), value->seed);
    } else if (value && value->type == SEED_RANGE) {
        txt.format("%s, [%g ... %g, %g]", CSTR(name), value->min, value->max, value->step);
    } else if (value) {
        txt.format("%s, ?", CSTR(name));
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
            listbox->setCurrentItem(j, TRUE);
            return 1;
        }
    }
    return 0;
}

void list_populate(FXList *list, fit_parameters *fps, seeds *seed, bool clear)
{
    if (clear) {
        list->clearItems();
    }
    for (size_t i = 0; i < fps->number; i++) {
        const fit_param_t *fp = &fps->values[i];
        const seed_t *value = (seed ? &seed->values[i] : NULL);
        list->appendItem(format_fit_parameter(fp, value));
    }
}
