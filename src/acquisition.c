#include <math.h>

#include "acquisition.h"
#include "fit-params.h"

double *
acquisition_parameter_pointer(struct acquisition_parameters *acquisition, int parameter_id)
{
    if (acquisition->type == SYSTEM_REFLECTOMETER) {
        if (parameter_id == PID_FIRSTMUL) {
            return &acquisition->parameters.sr.rmult;
        }
    } else if (acquisition->type == SYSTEM_ELLISS_AB) {
        if (parameter_id == PID_AOI) {
            return &acquisition->parameters.rpe.aoi;
        } else if (parameter_id == PID_ANALYZER) {
            return &acquisition->parameters.rpe.analyzer;
        }
    } else if (acquisition->type == SYSTEM_ELLISS_PSIDEL) {
        if (parameter_id == PID_AOI) {
            return &acquisition->parameters.se.aoi;
        }
    }
    return NULL;
}

int
acquisition_apply_param(struct acquisition_parameters *acquisition, int param_id, double val)
{
    double *parameter_ptr = acquisition_parameter_pointer(acquisition, param_id);
    if (!parameter_ptr) return 1;
    *parameter_ptr = val;
    return 0;
}

double
acquisition_get_parameter(const struct acquisition_parameters *acquisition, int param_id)
{
    const double *parameter_ptr = acquisition_parameter_pointer((struct acquisition_parameters *) acquisition, param_id);
    return (parameter_ptr ? *parameter_ptr : 0.0);
}

void
acquisition_get_all_parameters(const struct acquisition_parameters *acquisition, struct fit_parameters *fps)
{
    fit_param_t fp[1];
    if (acquisition->type == SYSTEM_REFLECTOMETER) {
        fp->id = PID_FIRSTMUL;
        fit_parameters_add(fps, fp);
    } else if (acquisition->type == SYSTEM_ELLISS_AB) {
        fp->id = PID_AOI;
        fit_parameters_add(fps, fp);
        fp->id = PID_ANALYZER;
        fit_parameters_add(fps, fp);
    } else if (acquisition->type == SYSTEM_ELLISS_PSIDEL) {
        fp->id = PID_AOI;
        fit_parameters_add(fps, fp);
    }
}

void
acquisition_set_default(struct acquisition_parameters *acquisition)
{
    acquisition->type = SYSTEM_REFLECTOMETER;
    acquisition->parameters.sr.rmult = 1.0;
}

void
acquisition_set_default_rpe(struct acquisition_parameters *acquisition)
{
    acquisition->type = SYSTEM_ELLISS_AB;
    acquisition->parameters.rpe.aoi = 65.0;
    acquisition->parameters.rpe.analyzer = 25.0;
}


void
acquisition_set_default_se(struct acquisition_parameters *acquisition)
{
    acquisition->type = SYSTEM_ELLISS_PSIDEL;
    acquisition->parameters.rpe.aoi = 65.0;
}

void
acquisition_parameter_to_string(str_t name, int parameter_id) {
    if (parameter_id == PID_FIRSTMUL) {
        str_copy_c(name, "RMult");
    } else if (parameter_id == PID_AOI) {
        str_copy_c(name, "AOI");
    } else if (parameter_id == PID_ANALYZER) {
        str_copy_c(name, "A");
    } else {
        str_copy_c(name, "###");
    }
}

const char *
acquisition_parameter_name(int param_id)
{
    if (param_id == PID_AOI) {
        return "aoi";
    } else if (param_id == PID_ANALYZER) {
        return "analyzer";
    } else if (param_id == PID_FIRSTMUL) {
        return "rmult";
    }
    return NULL;
}
