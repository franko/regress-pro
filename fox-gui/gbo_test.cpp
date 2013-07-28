#include <stdlib.h>
#include <math.h>
#include <nlopt.h>

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

class spectra_list_ref {
public:
    spectra_list_ref(vector_owner<spectra_entry>& ls) : m_list(ls) {}
    double operator [] (unsigned k) const { return m_list[k].ref_value; }
    unsigned size() const { return m_list.size(); }
private:
    const vector_owner<spectra_entry>& m_list;
};

struct gbo_score_eval {
    virtual double eval(gsl_matrix* results) = 0;
    virtual ~gbo_score_eval() {}
};

#if 0
template <typename ReferenceArray>
class gbo_score_simple : public gbo_score_eval
{
public:
    gbo_score_simple(const ReferenceArray& ref) : m_ref(ref) { }

    virtual double eval(gsl_vector* results) {
        const unsigned n = m_ref.size();
        double p = 0.0;
        for (unsigned k = 0; k < n; k++) {
            const double del = gsl_vector_get(results, k) - m_ref[k];
            p = p + del * del;
        }
        return p;
    }
private:
    const ReferenceArray& m_ref;
};

struct regress_result {
    double a, b;
    double rsq;
};

template <typename ReferenceArray>
class gbo_score_correl : public gbo_score_eval
{
public:
    gbo_score_correl(const ReferenceArray& ref) : m_ref(ref) {
        prepare();
    }

    void linfit(gsl_vector* results, regress_result& r) {
        const unsigned n = m_ref.size();
        double xy_sum = 0.0, y_sum = 0.0;
        for (unsigned k = 0; k < n; k++) {
            const double x = m_ref[k], y = gsl_vector_get(results, k);
            y_sum += y;
            xy_sum += x * y;
        }
        const double den = n * m_xsqsum - m_xsum * m_xsum;
        r.b = (n * xy_sum - m_xsum * y_sum) / den;
        r.a = (m_xsqsum * y_sum - m_xsum * xy_sum) / den;
        double rsq = 0.0;
        for (unsigned k = 0; k < n; k++) {
            const double x = m_ref[k], y = gsl_vector_get(results, k);
            const double del = (r.b * x + r.a) - y;
            rsq += del * del;
        }
        r.rsq = rsq;
    }

    void prepare() {
        const unsigned n = m_ref.size();
        double x_sum = 0.0, xsq_sum = 0.0;
        for (unsigned k = 0; k < n; k++) {
            const double x = m_ref[k];
            x_sum += x;
            xsq_sum += x * x;
        }
        m_xsum = x_sum;
        m_xsqsum = xsq_sum;
    }

    virtual double eval(gsl_vector* results) {
        regress_result reg;
        linfit(results, reg);
        fprintf(stderr, "REG: a=%g b=%g Rsq=%g\n", reg.a, reg.b, reg.rsq);
        return reg.rsq;
    }

private:
    const ReferenceArray& m_ref;
    double m_xsum, m_xsqsum;
};
#endif

template <typename ReferenceArray, bool AddError>
class gbo_score_offset : public gbo_score_eval
{
public:
    gbo_score_offset(const ReferenceArray& ref) : m_ref(ref) { }

    virtual double eval(gsl_matrix* results) {
        const unsigned n = m_ref.size();
        double del_s = 0.0;
        for (unsigned k = 0; k < n; k++) {
            const double x = m_ref[k], y = gsl_matrix_get(results, k, 0);
            del_s += (y - x);
        }
        const double del_avg = del_s / n;
        double res = 0.0;
        for (unsigned k = 0; k < n; k++) {
            const double x = m_ref[k], y = gsl_matrix_get(results, k, 0);
            res += (y - x - del_avg) * (y - x - del_avg);
            if (AddError) {
                const double stdev = gsl_matrix_get(results, k, 1);
                const double chi0 = gsl_matrix_get(results, k, 2);
                res += stdev * chi0;
            }
        }
        return res;
    }

private:
    const ReferenceArray& m_ref;
};

struct gbo_data {
    batch_engine* eng;
    gsl_matrix* results;
    gbo_score_eval* score;
};

double gbo_obj_func(unsigned n, const double *x, double *grad, void *my_func_data)
{
    gbo_data* gbo = (gbo_data *) my_func_data;
    batch_engine* eng = gbo->eng;
    fprintf(stderr, "X:");
    for (unsigned k = 0; k < n; k++) {
        fprintf(stderr, " %g", x[k]);
    }
    fprintf(stderr, "\n");
    gsl_vector_const_view xv = gsl_vector_const_view_array(x, n);
    eng->apply_goal_parameters(&xv.vector);
    eng->fit(gbo->results, 0);
    double score = gbo->score->eval(gbo->results);
    fprintf(stderr, "score: %g\n", score);
    return score;
}

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

    const unsigned N = list.size();

    struct fit_parameters* fps = fit_parameters_new();
    fit_param_t fp[1];
    set_model_param(fp, 2, MODEL_HO, 0);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 2, MODEL_HO, 5  + 0);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 2, MODEL_HO, 5  + 2);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 2, MODEL_HO, 5  + 4);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 2, MODEL_HO, 10 + 0);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 2, MODEL_HO, 10 + 2);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 2, MODEL_HO, 10 + 4);
    fit_parameters_add(fps, fp);

    set_model_param(fp, 3, MODEL_HO, 0  + 0);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 3, MODEL_HO, 0  + 2);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 3, MODEL_HO, 0  + 4);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 3, MODEL_HO, 5  + 0);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 3, MODEL_HO, 5  + 2);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 3, MODEL_HO, 5  + 4);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 3, MODEL_HO, 10 + 0);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 3, MODEL_HO, 10 + 2);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 3, MODEL_HO, 10 + 4);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 3, MODEL_HO, 15 + 0);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 3, MODEL_HO, 15 + 2);
    fit_parameters_add(fps, fp);
    set_model_param(fp, 3, MODEL_HO, 15 + 4);
    fit_parameters_add(fps, fp);

    batch_engine eng(fit, seeds, fps);
    spectra_source s_source(list);
    eng.init(s_source);
    eng.prefit();

    spectra_list_ref ref_array(list);
    gsl_matrix* fit_results = gsl_matrix_alloc(N, 3);
    gbo_score_eval* s_eval = new gbo_score_offset<spectra_list_ref, true>(ref_array);
    gbo_data gdata = {&eng, fit_results, s_eval};

    double x[32];

    for (unsigned k = 0; k < fps->number; k++) {
        x[k] = fit_engine_get_parameter_value(fit, &fps->values[k]);
    }

    double lb[19];
    double ub[19];

    for (unsigned k = 0; k < fps->number; k++) {
        get_param_bounds(&fps->values[k], lb + k, ub + k);
    }

    nlopt_opt opt = nlopt_create(NLOPT_LN_BOBYQA, fps->number);
    nlopt_set_lower_bounds(opt, lb);
    nlopt_set_upper_bounds(opt, ub);
    nlopt_set_min_objective(opt, gbo_obj_func, &gdata);
    nlopt_set_xtol_rel(opt, 1e-4);

    double minf;
    if (nlopt_optimize(opt, x, &minf) < 0) {
        printf("nlopt failed!\n");
    }
    else {
        printf("found minimum at f(%g,%g) = %0.10g\n", x[0], x[1], minf);
    }

    return true;
}
