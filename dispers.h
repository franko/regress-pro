#ifndef DISPERS_H
#define DISPERS_H

#include "cmpl.h"
#include "fit-params.h"
#include "str.h"
#include "disp-table.h"
#include "disp-sample-table.h"
#include "disp-bruggeman.h"
#include "disp-ho.h"
#include "disp-lookup.h"
#include "disp-cauchy.h"
#include "dispers-classes.h"

__BEGIN_DECLS

struct disp_struct;

struct disp_class {
    int disp_class_id;

    const char *short_id;
    const char *full_id;

    /* redundant with disp_class_id, should disappear */
    enum disp_model_id model_id;

    /* methods to copy and free dispersions */
    void (*free)(struct disp_struct *d);
    struct disp_struct *(*copy)(const struct disp_struct *d);

    cmpl(*n_value)(const struct disp_struct *d, double lam);
    int (*fp_number)(const struct disp_struct *d);
    cmpl(*n_value_deriv)(const struct disp_struct *d, double lam,
                         cmpl_vector *der);
    int (*apply_param)(struct disp_struct *d, const fit_param_t *fp,
                       double val);
    double(*get_param_value)(const struct disp_struct *d,
                             const fit_param_t *fp);

    /* class methods */
    int (*decode_param_string)(const char *param);
    void (*encode_param)(str_t param, const fit_param_t *fp);
};

struct deriv_info {
    int is_valid;
    cmpl_vector *val;
};

enum disp_type {
    DISP_UNSET = 0,
    DISP_TABLE,
    DISP_SAMPLE_TABLE,
    DISP_CAUCHY,
    DISP_HO,
    DISP_LOOKUP,
    DISP_BRUGGEMAN,
    DISP_END_OF_TABLE, /* Not a dispersion type */
};

struct disp_struct {
    struct disp_class *dclass;
    enum disp_type type;
    str_t name;
    union {
        struct disp_table table;
        struct disp_sample_table sample_table;
        struct disp_cauchy cauchy;
        struct disp_ho ho;
        struct disp_lookup lookup;
        struct disp_bruggeman bruggeman;
    } disp;
};

typedef struct disp_struct disp_t;

extern disp_t * load_nk_table(const char * filename);
extern disp_t * load_mat_dispers(const char * filename);
extern cmpl     n_value(const disp_t *mat, double lam);
extern void     n_value_cpp(const disp_t *mat, double lam,
                            double *nr, double *ni);
extern void     n_value_deriv(const disp_t *disp, cmpl_vector *der,
                              double lambda);
extern int      dispers_apply_param(disp_t *d, const fit_param_t *fp,
                                    double val);
extern void     get_model_param_deriv(const disp_t *d,
                                      struct deriv_info *der,
                                      const fit_param_t *fp, double lambda,
                                      double *dnr, double *dni);
extern void     disp_free(disp_t *d);
extern disp_t * disp_copy(disp_t *d);
extern disp_t * disp_new(enum disp_type tp);
extern disp_t * disp_new_with_name(enum disp_type tp, const char *name);
extern int      disp_get_number_of_params(const disp_t *d);
extern double   disp_get_param_value(const disp_t *d, const fit_param_t *fp);
extern int      disp_integrity_check(disp_t *d);
extern int      disp_get_model_id(disp_t *d);
extern int      disp_check_fit_param(disp_t *d, fit_param_t *fp);

int             decode_fit_param(fit_param_t *fp, const str_t str);

extern disp_t * disp_base_copy(const disp_t *src);
extern void     disp_base_free(disp_t *d);
extern int      disp_base_decode_param_string(const char *param);
extern int      disp_base_fp_number(const disp_t *src);

__END_DECLS

#endif
