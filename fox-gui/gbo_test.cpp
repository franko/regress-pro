#include <stdlib.h>

#include "str.h"
#include "batch_engine.h"
#include "spectra.h"

struct spectra_entry {
    str_ptr filename;
    double ref_value;

    void dispose() {
        STR_PTR_FREE(filename);
    }
};

class spectra_source : public generator<struct spectrum*> {
public:
    spectra_source(vector_owner<spectra_entry>& list): m_list(list), m_index(0) {}

    virtual void reset() { m_index = 0; }

    virtual generator_e next(struct spectrum*& v) {
        if (m_index < m_list.size()) {
            spectra_entry entry = m_list[m_index ++];
            v = load_gener_spectrum(CSTR(entry.filename));
            return (v ? generator_value : generator_error);
        }
        return generator_end;
    }

private:
    vector_owner<spectra_entry>& m_list;
    unsigned m_index;
};

bool gbo_test(struct fit_engine* fit, struct seeds *seeds, const char* tab_filename)
{
    FILE* f = fopen(tab_filename, "r");
    if (!f) {
        return false;
    }
    
    vector_owner<spectra_entry> list;
    str_t line;
    str_init(line, 64);
    for (int status = str_getline(line, f); status >= 0; status = str_getline(line, f)) {
        const char *cline = CSTR(line);
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
    str_free(line);

    batch_engine eng(fit, seeds);
    spectra_source s_source(list);
    eng.init(s_source);
    eng.prefit();

    gsl_vector* results = gsl_vector_alloc(list.size());
    eng.fit(results, 0);
    return true;
}
