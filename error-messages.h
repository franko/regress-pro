#ifndef ERROR_MESSAGES_H
#define ERROR_MESSAGES_H

#include "defs.h"
#include "str.h"

__BEGIN_DECLS

enum error_type_e {
    LOADING_FILE_ERROR,
    RECIPE_CHECK,
    FIT_ERROR
};

extern str_ptr new_error_message(enum error_type_e c, const char * msg, ...);
extern void    free_error_message(str_ptr error_msg);

__END_DECLS

#endif
