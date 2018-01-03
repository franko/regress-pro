#include <time.h>
#include <stdio.h>

#include "fit-timestamp.h"

unsigned int fit_timestamp(unsigned int buffer_size, char *buffer) {
    time_t t = time(nullptr);
    struct tm tm = *localtime(&t);
    return strftime(buffer, buffer_size, "modified by fit on %d/%m/%Y at %H:%M:%S", &tm);
}
