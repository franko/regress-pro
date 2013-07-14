#include <agg2/agg_array.h>

#include "fit-engine.h"
#include "spectra.h"

struct spectra_list : agg::pod_bvector<struct spectrum*> {
    spectra_list() : agg::pod_bvector<struct spectrum*>() { }

    void dispose_all() {
        for(unsigned k = 0; k < size(); k++) {
            spectra_free(at(k));
        }
        clear();
    }

    ~spectra_list() {
        dispose_all();
    }
};

class batch_engine {
public:
    batch_engine() {}
    ~batch_engine() {}

    bool init(struct fit_config& conf, spectra_gen& gen);

private:
    spectra_list m_spectra;
    enum system_kind m_spectra_kind;
    struct fit_engine m_fit_engine[1];
};

bool batch_engine::init(struct fit_config& config, spectra_gen& gen)
{
    gen.reset();

    struct spectrum *s_first = gen.next();
    if(s_first == NULL) {
        return false;
    }

    *m_fit_engine->config = config;
    m_spectra_kind = fit_engine_batch_prepare(m_fit_engine, s_first);

    m_spectra.dispose_all();
    m_spectra.push_back(s_first);

    for(struct spectrum* s = gen.next(); s != 0; s = gen.next()) {
        m_spectra.push_back(s);
        if(s->config.system != m_spectra_kind) {
            return false;
        }
    }
}
