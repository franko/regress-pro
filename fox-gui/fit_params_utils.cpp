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
