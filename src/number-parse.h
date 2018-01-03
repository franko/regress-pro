#ifndef NUMBER_PARSE_H
#define NUMBER_PARSE_H

#include "defs.h"

enum {
    PARSE_FLOAT_SKIP_SPACES   = 1 << 0,
    PARSE_FLOAT_FRENCH_LOCALE = 1 << 1,
};

extern const char *skip_spaces(const char *s);
extern int parse_float(const char *text, unsigned int flags, float *value, int *n_parsed);

#endif
