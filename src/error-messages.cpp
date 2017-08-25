#include "error-messages.h"

static const char * error_type_id[] = {
    "Error reading file.",
    "Error checking recipe.",
    "Error running script.",
    "Spectrum not found.",
    "Error running fit.",
};

str_ptr
new_error_message(enum error_type_e etype, const char * msg, ...)
{
    str_ptr error_msg = str_new();
    str_t tail;
    va_list ap;

    str_init_from_c(error_msg, error_type_id[etype]);

    va_start(ap, msg);
    str_init(tail, 64);
    str_vprintf(tail, msg, 0, ap);
    str_append(error_msg, tail, ' ');
    str_free(tail);
    va_end(ap);

    return error_msg;
}

void
free_error_message(str_ptr error_msg)
{
    str_free(error_msg);
    free(error_msg);
}
