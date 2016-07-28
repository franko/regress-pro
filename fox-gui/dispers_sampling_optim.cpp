#include <algorithm>
#include <cmath>

#include <gsl/gsl_interp.h>

#include "dispers_sampling_optim.h"

template <typename T, unsigned N>
struct tab_array {
    tab_array(int size): m_size(size), m_array(N * size) { }

    int size() const { return m_size; }

    const double *row(int k) const { return m_array.data() + k * m_size; }
          double *row(int k)       { return m_array.data() + k * m_size; }

    const T& at(int i, int j) const { return m_array[i + m_size * j]; }
          T& at(int i, int j)       { return m_array[i + m_size * j]; }

private:
    int m_size;
    pod_array<double> m_array;
};

class cspline_array_interp {
public:
    using double_pair = std::pair<double, double>;

    cspline_array_interp(const tab_array<double, 3>& array)
    : m_table(array) {
        const int length = m_table.size();
        m_interp_n = gsl_interp_alloc(gsl_interp_cspline, length);
        m_interp_k = gsl_interp_alloc(gsl_interp_cspline, length);
        m_accel = gsl_interp_accel_alloc();
        compute_interpolation();
    }

    ~cspline_array_interp() {
        gsl_interp_free(m_interp_n);
        gsl_interp_free(m_interp_k);
        gsl_interp_accel_free(m_accel);
    }

    void compute_interpolation() {
        gsl_interp_init(m_interp_n, m_table.row(0), m_table.row(1), m_table.size());
        gsl_interp_init(m_interp_k, m_table.row(0), m_table.row(2), m_table.size());
    }

    double_pair eval(const double wavelength) const {
        const double n = gsl_interp_eval(m_interp_n, m_table.row(0), m_table.row(1), wavelength, m_accel);
        const double k = gsl_interp_eval(m_interp_k, m_table.row(0), m_table.row(2), wavelength, m_accel);
        return double_pair(n, k);
    }
private:
    const tab_array<double, 3>& m_table;

    gsl_interp *m_interp_n;
    gsl_interp *m_interp_k;
    gsl_interp_accel *m_accel;
};

tab_array<double, 3> get_array(const pod_array<int> ipoints, const tab_matrix& ms) {
    tab_array<double, 3> table(ipoints.size());
    for (int i = 0; i < ipoints.size(); i++) {
        const int k = ipoints[i];
        table.at(i, 0) = ms(k, 0);
        table.at(i, 1) = ms(k, 1);
        table.at(i, 2) = ms(k, 2);
    }
    return table;
}

int subsampling_eval(const tab_matrix& ms, const pod_array<int>& ipoints, const double tol) {
    tab_array<double, 3> table = get_array(ipoints, ms);
    cspline_array_interp interp(table);
    for (int i = 0; i < ms.size(); i++) {
        auto nki = interp.eval(ms(i, 0));
        auto nkr = std::pair<double, double>(ms(i, 1), ms(i, 2));
        if (std::abs(nki.first  - nkr.first ) > tol ||
            std::abs(nki.second - nkr.second) > tol) {
            return i;
        }
    }
    return -1;
}

double interp_delta_score(const cspline_array_interp& interp, const tab_matrix& ms, const pod_array<int>& ipoints, const int i_min, const int i_max) {
    double del_n = 0, del_k = 0;
    for (int i = ipoints[i_min]; i < ipoints[i_max]; i++) {
        auto nki = interp.eval(ms(i, 0));
        auto nkr = std::pair<double, double>(ms(i, 1), ms(i, 2));
        del_n = std::max(del_n, std::abs(nki.first  - nkr.first ));
        del_k = std::max(del_k, std::abs(nki.second - nkr.second));
    }
    return std::max(del_n, del_k);
}

std::pair<int, double> find_delta_optimal(const tab_matrix& ms, pod_array<int>& ipoints, int i, int ka, int kb) {
    int kbest = ka;
    double del = 1000.0;

    tab_array<double, 3> table = get_array(ipoints, ms);
    cspline_array_interp interp(table);

    for (int k = ka + 1; k < kb - 1; k++) {
        table.row(0)[i] = ms(k, 0);
        table.row(1)[i] = ms(k, 1);
        table.row(2)[i] = ms(k, 2);
        interp.compute_interpolation();
        double kdel = interp_delta_score(interp, ms, ipoints, i - 1, i + 1);
        if (kdel < del) {
            kbest = k;
            del = kdel;
        }
    }
    return std::pair<int, double>(kbest, del);
}

void insert_opt_point(const tab_matrix& ms, pod_array<int>& ipoints, int i) {
    int ka = ipoints[i], kb = ipoints[i+1];
    ipoints.insert(i + 1, ka + 1);
    auto best = find_delta_optimal(ms, ipoints, i + 1, ka, kb);
    ipoints[i+1] = best.first;
}

void add_new_point(const tab_matrix& ms, pod_array<int>& ipoints, int i_nok) {
    const double wi = ms(i_nok, 0);
    for (int i = 0; i < ipoints.size(); i++) {
        const double wa = ms(ipoints[i    ], 0);
        const double wb = ms(ipoints[i + 1], 0);
        if (wa < wi && wb > wi) {
            insert_opt_point(ms, ipoints, i);
            break;
        }
    }
}

pod_array<int> optimize_sampling_points(const rc_matrix* m, const double approx_tolerance) {
    const tab_matrix ms(m);

    pod_array<int> ipoints(2);
    ipoints[0] = 0;
    ipoints[1] = ms.size() - 1;

    add_new_point(ms, ipoints, ms.size() / 2);

    while (true) {
        int i_nok = subsampling_eval(ms, ipoints, approx_tolerance);
        if (i_nok < 0) break;
        add_new_point(ms, ipoints, i_nok);
    }

    return ipoints;
}

disp_t *dispersion_from_sampling_points(const tab_matrix& src, const pod_array<int>& ipoints, const char *name) {
    disp_t *new_disp = disp_new(DISP_SAMPLE_TABLE);
    disp_set_name(new_disp, name);
    disp_sample_table * const st_disp = &new_disp->disp.sample_table;
    const int sampling_points = ipoints.size();
    disp_sample_table_init(st_disp, sampling_points);
    gsl_matrix * const st_matrix = &st_disp->table->view.matrix;
    for (int i = 0; i < sampling_points; i++) {
        st_matrix->data[i                     ] = src(ipoints[i], 0);
        st_matrix->data[i + 1 * st_matrix->tda] = src(ipoints[i], 1);
        st_matrix->data[i + 2 * st_matrix->tda] = src(ipoints[i], 2);
    }
    disp_sample_table_prepare_interp(st_disp);
    return new_disp;
}
