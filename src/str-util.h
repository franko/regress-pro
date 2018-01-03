#ifndef STR_UTIL_H
#define STR_UTIL_H

#include "defs.h"
#include "str.h"

extern int      str_getcwd(str_t dir);
extern int      str_loadfile(const char *filename, str_t text);
extern int      str_is_abs_pathname(str_t path);
extern int      str_path_basename(str_ptr basename, const char *filename);

#endif
