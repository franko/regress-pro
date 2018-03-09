#include <stdio.h>
#include <stdlib.h>

#include "disp-load-binary-dat.h"
#include "error-messages.h"

disp_t *disp_binary_dat_load(const char *filename, str_ptr *error_msg) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        *error_msg = new_error_message(LOADING_FILE_ERROR, "File \"%s\" does not exists or cannot be opened", filename);
        return nullptr;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    str file_content(fsize);
    fread(file_content.heap, fsize, 1, f);
    fclose(f);

    file_content.heap[fsize] = 0;
    file_content.length = fsize;

    *error_msg = new_error_message(LOADING_FILE_ERROR, "Open DAT file not yet implemented");
    return nullptr;
}
