#ifndef FIT_PARAMS_UTILS_H
#define FIT_PARAMS_UTILS_H

#include <fx.h>
#include "stack.h"

/* Create a fit_parameters list with all the fit parameters for the given stack and
   populate the listbox with the corresponding entries.
   For each entry of the listbox the corresponding index in the fit_parameters list
   it is provided as a user-data. */
extern fit_parameters *listbox_populate_all_parameters(FXListBox *listbox, stack_t *stack, const struct acquisition_parameters *acquisition);
extern FXString format_fit_parameter(const fit_param_t *fp, const seed_t *value = NULL);
extern int listbox_select_parameter(FXListBox *listbox, int fp_index);
extern void list_populate(FXList *list, fit_parameters *fps, seeds *value, bool clear);
extern FXMenuPane *fit_parameters_menu(FXWindow *win, FXObject *target, FXSelector sel, fit_parameters *fps);

#endif
