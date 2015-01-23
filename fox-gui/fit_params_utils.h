#ifndef FIT_PARAMS_UTILS_H
#define FIT_PARAMS_UTILS_H

#include <fx.h>
#include "stack.h"

/* Create a fit_parameters list with all the fit parameters for the given stack and
   populate the listbox with the corresponding entries.
   For each entry of the listbox the corresponding index in the fit_parameters list
   it is provided as a user-data. */
extern fit_parameters *listbox_populate_all_parameters(FXListBox *listbox, stack_t *stack);

#endif
