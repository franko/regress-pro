#ifndef DISPERS_SAMPLING_OPTIM_H
#define DISPERS_SAMPLING_OPTIM_H

#include "dispers.h"
#include "rc_matrix.h"
#include "pod_array.h"

extern pod_vector<int> optimize_sampling_points(const disp_sample_table *disp, const double approx_tolerance);
extern disp_t *dispersion_from_sampling_points(const disp_sample_table *disp, const pod_vector<int>& ipoints, const char *name);
extern disp_t *dispersion_from_sampling_points(const disp_t *disp, const pod_vector<double>& sampling_points, const char *name);
extern pod_vector<double> optimize_sampling_points_c(const disp_t *disp, const double approx_tolerance);

#endif
