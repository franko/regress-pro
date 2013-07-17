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

template <typename T>
struct generator {
    virtual void reset() = 0;
    virtual T next() = 0;
    virtual ~generator() { }
};

class batch_engine {
public:
    batch_engine() {}
    ~batch_engine() {}

    bool init(generator<struct spectrum*>& gen);
    bool prefit();
    void apply_goal_parameters(const struct fit_parameters *fps, const gsl_vector *x);
    void fit(gsl_vector* results, int output_param);

private:
    vector_owner<spectrum_item> m_spectra;
    enum system_kind m_spectra_kind;
    struct fit_engine* m_fit_engine;
    struct seeds* m_seeds;
};

#endif

