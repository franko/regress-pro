#ifndef STR_UTIL_H
#define STR_UTIL_H

#include "defs.h"
#include "str.h"

__BEGIN_DECLS

extern int      str_getcwd          (str_t dir);
extern int      str_loadfile        (const char *filename, str_t text);
extern int      str_is_abs_pathname (str_t path);

__END_DECLS

#endif
