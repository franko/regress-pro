#include <stdlib.h>

#include "batch_engine.h"

struct spectra_entry {
    str_ptr filename;
    double ref_value;

    void dispose() {
        STR_PTR_FREE(filename);
    }
};

class spectra_source : generator<struct spectrum*> {
public:
    spectra_source() ####
};

bool gbo_test(const char* tab_filename)
{
    FILE* f = fopen(tab_filename, "r");
    if (!f) {
        return false;
    }
    
    vector_owner<spectra_entry> list;
    str_t line;
    for (int status = str_getline(line, f); status >= 0; status = str_getline(line, f)) {
        const char *cline = cline;
        const char *p = strchr(cline, int(','));
        if (!p) {
            return false;
        }
        spectra_entry entry;
        entry.filename = str_new();
        str_copy_c_substr(entry.filename, cline, p - cline);

        char* tail;
        entry.ref_value = strtod(p + 1, &tail);
        if (tail == p+1) {
            return false;
        }

        list.add(entry);
    }
}
