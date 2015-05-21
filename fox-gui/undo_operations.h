#ifndef UNDO_OPERATIONS_H
#define UNDO_OPERATIONS_H

#include <agg2/agg_array.h>

#include "fit-params.h"
#include "fit_manager.h"

struct fit_action {
    virtual void apply(fit_manager *fm) = 0;
    virtual void undo(fit_manager *fm) = 0;
    virtual ~fit_action() {}
};

struct oper_set_parameter : fit_action {
    oper_set_parameter(unsigned k, double oval, double nval):
        index(k), old_value(oval), new_value(nval)
    {}
    virtual void apply(fit_manager *fm) {
        fm->set_parameter_value(index, new_value);
    }
    virtual void undo(fit_manager *fm) {
        fm->set_parameter_value(index, old_value);
    }
    unsigned index;
    double old_value, new_value;
};

struct oper_set_sampling : fit_action {
    oper_set_sampling(double os, double oe, double op, double ns, double ne, double np):
        old_start(os), old_end(oe), old_step(op),
        new_start(ns), new_end(ne), new_step(np)
    {}
    virtual void apply(fit_manager *fm) {
        fm->set_sampling(new_start, new_end, new_step);
    }
    virtual void undo(fit_manager *fm) {
        fm->set_sampling(old_start, old_end, old_step);
    }
    double old_start, old_end, old_step;
    double new_start, new_end, new_step;
};

struct oper_run_fit : fit_action {
    oper_run_fit(fit_parameters *fps, double *ovals, double *nvals):
        params(fit_parameters_copy(fps)), old_values(ovals), new_values(nvals)
    { }

    ~oper_run_fit() {
        delete [] old_values;
        delete [] old_values;
        fit_parameters_free(params);
    }

    void apply_values(fit_manager *fm, double *values) {
        for (unsigned i = 0; i < params->number; i++) {
            int k = fm->lookup(&params->values[i]);
            if (k >= 0) {
                fm->set_parameter_value(k, values[i]);
            }
        }
    }

    virtual void apply(fit_manager *fm) {
        apply_values(fm, new_values);
    }

    virtual void undo(fit_manager *fm) {
        apply_values(fm, old_values);
    }

    fit_parameters *params;
    double *old_values, *new_values;
};

class actions_record {
private:
    agg::pod_bvector<fit_action*> m_records;
    unsigned m_cursor;
public:
    actions_record(): m_cursor(0) { }

    void record(fit_action *action) {
        m_records.add(action);
    }
};

#endif
