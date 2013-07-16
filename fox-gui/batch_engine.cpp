#include <agg2/agg_array.h>

#include "fit-engine.h"
#include "fit-params.h"
#include "spectra.h"
#include "grid-search.h"
#include "Strcpp.h"

struct spectrum_item {
    struct spectrum* self;
    gsl_vector* seeds;

    void assign(struct spectrum* s, int nparam) {
        self = s;
        seeds = gsl_vector_free(nparam);
    }

    void dispose() {
        spectra_free(self);
        gsl_vector_free(seeds);
    }
};

struct spectra_list : agg::pod_bvector<spectrum_item> {
    spectra_list() : agg::pod_bvector<spectrum_item>() { }

    void dispose_all() {
        for(unsigned k = 0; k < size(); k++) {
            at(k).dispose();
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
    struct fit_engine* m_fit_engine;
    struct seeds* m_seeds;
};

bool batch_engine::init(spectra_gen& gen)
{
    gen.reset();

    struct spectrum *s_first = gen.next();
    if(s_first == NULL) {
        return false;
    }

    m_spectra_kind = fit_engine_batch_prepare(m_fit_engine, s_first);

    m_spectra.dispose_all();
    m_spectra.push_back(s_first);

    for(struct spectrum* s = gen.next(); s != 0; s = gen.next()) {
        if(s->config.system != m_spectra_kind) {
            return false;
        }
        spectrum_item it;
        fit_engine_prepare_spectrum(m_fit_engine, s);
        it.assign(s, fit->parameters->number);
        m_spectra.push_back(it);
    }
    return true;
}

bool batch_engine::prefit()
{
    for(unsigned k = 0; k < m_spectra.size(); k++) {
        spectrum_item it = m_spectra[k];
        fit_engine_attach_spectrum(m_fit_engine, it.self);

        double chisq;
        Str error_msgs, analysis;
        lmfit_grid(m_fit_engine, m_seeds, &chisq, analysis.str(), error_msgs.str(),
                   LMFIT_PRESERVE_STACK, NULL, NULL);

        gsl_vector_memcpy(it.seeds, m_fit_engine->run->results);
    }
}

void batch_engine::apply_goal_parameters(const struct fit_parameters *fps, const gsl_vector *x)
{
    fit_engine_apply_parameters(m_fit_engine, fps, x);
}

gsl_vector* batch_engine::fit(int output_param)
{
    const int max_iter = 20;
    const gsl_multifit_fdfsolver_type *T = gsl_multifit_fdfsolver_lmsder;
    struct fit_config *cfg = fit->config;
    int np = fit->parameters->number;
    gsl_vector *x = gsl_vector_alloc(np);
    gsl_vector *results = gsl_vector_alloc(m_spectra.size());

    for (unsigned k = 0; k < m_spectra.size(); k++) {
        const spectrum_item& si = m_spectra[k];
        gsl_multifit_function_fdf *f;
        gsl_multifit_fdfsolver *solver;
        int nb_iter;
        fit_engine_attach_spectrum(fit, si.self);
        f = &fit->run->mffun;
        solver = gsl_multifit_fdfsolver_alloc(T, f->n, f->p);
        gsl_vector_memcpy(x, si.seeds);
        lmfit_iter(x, f, solver, max_iter, cfg->epsabs, cfg->epsrel, &nb_iter, NULL, NULL, NULL);
        gsl_vector_set(results, k, gsl_vector_get(x, output_param));
        gsl_multifit_fdfsolver_free(solver);
    }

    return results;
}
