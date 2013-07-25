#ifndef BATCH_ENGINE_H
#define BATCH_ENGINE_H

#include <agg2/agg_array.h>
#include <gsl/gsl_vector.h>

#include "fit-engine.h"
#include "fit-params.h"
#include "spectra.h"

struct spectrum_item {
    struct spectrum* self;
    gsl_vector* seeds;

    void assign(struct spectrum* s, int nparam) {
        self = s;
        seeds = gsl_vector_alloc(nparam);
    }

    void dispose() {
        spectra_free(self);
        gsl_vector_free(seeds);
    }
};

// T should be a POD object
template <typename T>
struct vector_owner : agg::pod_bvector<T> {
    vector_owner() : agg::pod_bvector<T>() { }

    void dispose_all() {
        for(unsigned k = 0; k < this->size(); k++) {
            this->at(k).dispose();
        }
        this->clear();
    }

    ~vector_owner() {
        dispose_all();
    }
};

enum generator_e {
    generator_end = 0,
    generator_value = 1,
    generator_error = -1,
};

template <typename T>
struct generator {
    virtual void reset() = 0;
    virtual generator_e next(T& v) = 0;
    virtual ~generator() { }
};

class batch_engine {
public:
    batch_engine(struct fit_engine* fit, struct seeds* seeds, const struct fit_parameters *gbo_fps) :
        m_fit_engine(fit), m_seeds(seeds), m_gbo_params(gbo_fps)
    {}

    ~batch_engine() {}

    bool init(generator<struct spectrum*>& gen);
    void prefit();
    void apply_goal_parameters(const gsl_vector *x);
    void fit(gsl_vector* results, gsl_vector* cov_results, int output_param);

private:
    vector_owner<spectrum_item> m_spectra;
    enum system_kind m_spectra_kind;
    struct fit_engine* m_fit_engine;
    struct seeds* m_seeds;
    const struct fit_parameters* m_gbo_params;
};

#endif

