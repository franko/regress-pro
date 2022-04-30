#ifndef UNDO_OPERATIONS_H
#define UNDO_OPERATIONS_H

#include <agg_array.h>

#include "fit-params.h"
#include "fit_manager.h"

struct fit_action {
    virtual void apply(fit_manager *fm) = 0;
    virtual void undo(fit_manager *fm) = 0;
    virtual int class_id() const = 0;
    virtual bool fuse(fit_action *that) { return false; }
    virtual ~fit_action() {}
};

enum {
    action_set_parameter,
    action_set_sampling,
    action_run_fit,
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
    virtual bool fuse(fit_action *_that) {
        oper_set_parameter *that = (oper_set_parameter *) _that;
        if (index == that->index) {
            new_value = that->new_value;
            return true;
        }
        return false;
    }
    virtual int class_id() const { return action_set_parameter; }
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
    virtual bool fuse(fit_action *_that) {
        oper_set_sampling *that = (oper_set_sampling *) _that;
        new_start = that->new_start;
        new_end = that->new_end;
        new_step = that->new_step;
        return true;
    }
    virtual int class_id() const { return action_set_sampling; }
    double old_start, old_end, old_step;
    double new_start, new_end, new_step;
};

struct oper_run_fit : fit_action {
    oper_run_fit(fit_parameters *fps, double *ovals, double *nvals):
        params(fit_parameters_copy(fps)), old_values(ovals), new_values(nvals)
    { }

    ~oper_run_fit() {
        delete [] old_values;
        delete [] new_values;
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

    virtual int class_id() const { return action_run_fit; }

    fit_parameters *params;
    double *old_values, *new_values;
};

static bool fuse_actions(fit_action *a, fit_action *b)
{
    if (a->class_id() == b->class_id()) {
        return a->fuse(b);
    }
    return false;
}

class actions_record {
private:
    agg::pod_bvector<fit_action*> m_records;
    unsigned m_cursor;
public:
    actions_record(): m_cursor(0) { }

    ~actions_record() {
        clear_tail(0);
    }

    void clear_tail(unsigned pos) {
        for (unsigned i = pos; i < m_records.size(); i++) {
            delete m_records[i];
        }
        m_records.free_tail(pos);
    }

    bool has_undo() { return (m_cursor > 0); }
    bool has_redo() { return (m_cursor < m_records.size()); }

    void clear() {
        clear_tail(0);
        m_cursor = 0;
    }

    void record(fit_action *action) {
        clear_tail(m_cursor);
        if (m_cursor > 0 && fuse_actions(m_records[m_cursor - 1], action)) {
            delete action;
        } else {
            m_records.add(action);
            m_cursor ++;
        }
    }

    bool undo(fit_manager *fm) {
        if (m_cursor == 0) return false;
        m_cursor --;
        m_records[m_cursor]->undo(fm);
        return true;
    }

    bool redo(fit_manager *fm) {
        if (m_cursor >= m_records.size()) return false;
        m_records[m_cursor]->apply(fm);
        m_cursor ++;
        return true;
    }
};

#endif
