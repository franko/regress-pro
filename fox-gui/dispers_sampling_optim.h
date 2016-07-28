#ifndef DISPERS_SAMPLING_OPTIM_H
#define DISPERS_SAMPLING_OPTIM_H

#include "dispers.h"
#include "rc_matrix.h"
#include "pod_array.h"

/* Provide accessors for elements from rc_matrix.
   We assume that data are stored in column-major form. */
class tab_matrix {
public:
    tab_matrix(const rc_matrix *m): m_matrix(m) { }

    int size() const { return m_matrix->view.matrix.size2; }
    double operator()(int i, int j) const { return m_matrix->view.matrix.data[j * m_matrix->view.matrix.tda + i]; }
private:
    const rc_matrix *m_matrix;
};

extern pod_array<int> optimize_sampling_points(const rc_matrix* m, const double approx_tolerance);
extern disp_t *dispersion_from_sampling_points(const tab_matrix& src, const pod_array<int>& ipoints, const char *name);
extern disp_t *dispersion_from_sampling_points(const disp_t *disp, const pod_vector<double>& sampling_points, const char *name);
extern pod_vector<double> optimize_sampling_points_c(const disp_t *disp, const double approx_tolerance);

#endif
