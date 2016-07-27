#ifndef DISPERS_SAMPLING_OPTIM_H
#define DISPERS_SAMPLING_OPTIM_H

#include "rc_matrix.h"
#include "pod_array.h"

pod_array<int> optimize_sampling_points(const rc_matrix* m, const double approx_tolerance);

#endif
