#include <math.h>

#include "acquisition.h"
#include "fit-params.h"
#include "elliss.h"

void
acquisition_set_zero(struct acquisition_parameters *acq)
{
    acq->type = SYSTEM_UNDEF;
    acq->numap = 0.0;
    acq->bandwidth = 0.0;
}

double *
acquisition_parameter_pointer(struct acquisition_parameters *acquisition, int parameter_id)
{
    if (parameter_id == PID_BANDWIDTH) {
        return &acquisition->bandwidth;
    }
    if (parameter_id == PID_NUMAP) {
        return &acquisition->numap;
    }
    const enum system_kind acq_type = acquisition->type;
    if (acq_type == SYSTEM_SR) {
        if (parameter_id == PID_FIRSTMUL) {
            return &acquisition->parameters.sr.rmult;
        }
    } else if (acq_type == SYSTEM_SE_RPE) {
        if (parameter_id == PID_AOI) {
            return &acquisition->parameters.rpe.aoi;
        } else if (parameter_id == PID_ANALYZER) {
            return &acquisition->parameters.rpe.analyzer;
        }
    } else if (acq_type == SYSTEM_SE_RAE) {
        if (parameter_id == PID_AOI) {
            return &acquisition->parameters.rpe.aoi;
        } else if (parameter_id == PID_POLARIZER) {
            /* RAE is aliased by RPE and analyzer = polarizer for RAE. */
            return &acquisition->parameters.rpe.analyzer;
        }
    } else if (acq_type == SYSTEM_SE) {
        if (parameter_id == PID_AOI) {
            return &acquisition->parameters.se.aoi;
        }
    }
    return NULL;
}

double
acquisition_get_se_aoi(const struct acquisition_parameters *acquisition)
{
    if (SE_TYPE(acquisition->type) == SE_ALPHA_BETA) {
        return acquisition->parameters.rpe.aoi;
    } else if (acquisition->type == SYSTEM_SE) {
        return acquisition->parameters.se.aoi;
    }
    return 0.0;
}

double
acquisition_get_se_analyzer(const struct acquisition_parameters *acquisition)
{
    if (SE_TYPE(acquisition->type) == SE_ALPHA_BETA) {
        return acquisition->parameters.rpe.analyzer;
    }
    return 0.0;
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
    if (acquisition->type == SYSTEM_SR) {
        fp->id = PID_FIRSTMUL;
        fit_parameters_add(fps, fp);
    } else if (acquisition->type == SYSTEM_SE_RPE) {
        fp->id = PID_AOI;
        fit_parameters_add(fps, fp);
        fp->id = PID_ANALYZER;
        fit_parameters_add(fps, fp);
        fp->id = PID_NUMAP;
        fit_parameters_add(fps, fp);
    } else if (acquisition->type == SYSTEM_SE_RAE) {
        fp->id = PID_AOI;
        fit_parameters_add(fps, fp);
        fp->id = PID_POLARIZER;
        fit_parameters_add(fps, fp);
        fp->id = PID_NUMAP;
        fit_parameters_add(fps, fp);
    } else if (acquisition->type == SYSTEM_SE) {
        fp->id = PID_AOI;
        fit_parameters_add(fps, fp);
        fp->id = PID_NUMAP;
        fit_parameters_add(fps, fp);
    }
    fp->id = PID_BANDWIDTH;
    fit_parameters_add(fps, fp);
}

void
acquisition_set_default(struct acquisition_parameters *acquisition)
{
    acquisition->type = SYSTEM_SR;
    acquisition->parameters.sr.rmult = 1.0;
    acquisition->numap = 0.0;
    acquisition->bandwidth = 0.0;
}

void
acquisition_set_default_rpe(struct acquisition_parameters *acquisition)
{
    acquisition->type = SYSTEM_SE_RPE;
    acquisition->parameters.rpe.aoi = 65.0;
    acquisition->parameters.rpe.analyzer = 25.0;
}

void
acquisition_set_default_se(struct acquisition_parameters *acquisition)
{
    acquisition->type = SYSTEM_SE;
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
    } else if (parameter_id == PID_POLARIZER) {
        str_copy_c(name, "P");
    } else if (parameter_id == PID_BANDWIDTH) {
        str_copy_c(name, "bandwidth");
    } else if (parameter_id == PID_NUMAP) {
        str_copy_c(name, "NA");
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
    } else if (param_id == PID_POLARIZER) {
        return "polarizer";
    } else if (param_id == PID_FIRSTMUL) {
        return "rmult";
    } else if (param_id == PID_BANDWIDTH) {
        return "bandwidth";
    } else if (param_id == PID_NUMAP) {
        return "numap";
    }
    return NULL;
}
