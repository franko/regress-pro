#ifndef ERROR_MESSAGES_H
#define ERROR_MESSAGES_H

#include "defs.h"
#include "str.h"

/* NB: if anything is changed here a corresponding change should be made in
   error-messages.cpp. */
enum error_type_e {
    LOADING_FILE_ERROR,
    RECIPE_CHECK,
    TESTING_SCRIPT_ERROR,
    SPECTRA_NOT_FOUND,
    SPECTRUM_INVALID,
    FIT_ERROR,
};

extern str_ptr new_error_message(enum error_type_e c, const char * msg, ...);
extern void    free_error_message(str_ptr error_msg);

#endif
