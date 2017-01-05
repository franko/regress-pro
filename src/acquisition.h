#ifndef ACQUISITION_H
#define ACQUISITION_H

#include "defs.h"
#include "str.h"

#if 0
struct meas_acquisition;

struct meas_acquisition_class {
    int id;

    const char *full_name;
    const char *short_name;

    int      (*parameters_number)(const struct meas_acquisition *a);
    int      (*apply_param)      (struct meas_acquisition *a, const fit_param_t *fp, double val);
    double * (*map_param)        (struct meas_acquisition *a, int index);
    double   (*get_param_value)  (const struct meas_acquisition *a, const fit_param_t *fp);
};

struct meas_acquisition {
    struct meas_acquisition_class klass;
};
#endif

__BEGIN_DECLS

struct fit_parameters;

enum system_kind {
    SYSTEM_UNDEFINED = 0,
    SYSTEM_REFLECTOMETER,
    SYSTEM_ELLISS_AB,
    SYSTEM_ELLISS_PSIDEL,
    SYSTEM_EXCEED_VALUE
};

/* Ellipsometry parameters: */
struct rpe_acquisition_parameters {
    double aoi; /* Angle in degrees. */
    double analyzer;  /* Angle in degrees. */
};

/* Ellipsometry parameters: */
struct se_acquisition_parameters {
    double aoi; /* Angle in degrees. */
};

/* Reflectometry parameters */
struct sr_acquisition_parameters {
    double rmult;
};

struct acquisition_parameters {
    enum system_kind type;
    union {
        struct rpe_acquisition_parameters rpe;
        struct se_acquisition_parameters  se;
        struct sr_acquisition_parameters  sr;
    } parameters;
    double bandwidth;
};

extern void     acquisition_set_zero(struct acquisition_parameters *acq);
extern int      acquisition_apply_param(struct acquisition_parameters *acq, int param_id, double val);
extern void     acquisition_get_all_parameters(const struct acquisition_parameters *acq, struct fit_parameters *fps);
extern double   acquisition_get_parameter(const struct acquisition_parameters *acq, int param_id);
extern double * acquisition_parameter_pointer(struct acquisition_parameters *acq, int param_id);
extern void     acquisition_set_default(struct acquisition_parameters *acq);
extern void     acquisition_set_default_rpe(struct acquisition_parameters *acq);
extern void     acquisition_set_default_se(struct acquisition_parameters *acq);
extern void     acquisition_parameter_to_string(str_t name, int param_id);
extern double   acquisition_get_se_aoi(const struct acquisition_parameters *acquisition);
extern double   acquisition_get_se_analyzer(const struct acquisition_parameters *acquisition);

extern const char * acquisition_parameter_name(int param_id);

__END_DECLS

#endif
