#ifndef PARSER_H
#define PARSER_H

#include "str_cpp.h"

struct quoted_string {
    str text;
};

struct quoted_string_ref {
    const str& text;
};

#endif
