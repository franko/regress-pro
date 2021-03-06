#ifndef DISPERS_H
#define DISPERS_H

#include "cmpl.h"
#include "fit-params.h"
#include "str.h"
#include "dispers-classes.h"
#include "disp-table.h"
#include "disp-sample-table.h"
#include "disp-bruggeman.h"
#include "disp-ho.h"
#include "disp-fb.h"
#include "disp-lookup.h"
#include "disp-cauchy.h"
#include "dispers-classes.h"
#include "writer.h"
#include "lexer.h"

__BEGIN_DECLS

struct disp_struct;

struct disp_class {
    enum disp_type disp_class_id;

    const char *full_name;
    const char *short_name;

    /* methods to copy and free dispersions */
    void (*free)(struct disp_struct *d);
    struct disp_struct *(*copy)(const struct disp_struct *d);

    cmpl(*n_value)(const struct disp_struct *d, double lam);
    int (*fp_number)(const struct disp_struct *d);
    cmpl(*n_value_deriv)(const struct disp_struct *d, double lam,
                         cmpl_vector *der);
    int (*apply_param)(struct disp_struct *d, const fit_param_t *fp,
                       double val);
    double *(*map_param)(struct disp_struct *d, int index);
    double(*get_param_value)(const struct disp_struct *d,
                             const fit_param_t *fp);
    int (*write)(writer_t *w, const struct disp_struct *_d);

    /* class methods */
    void (*encode_param)(str_t param, const fit_param_t *fp);
    int (*read)(lexer_t *l, struct disp_struct *d);
};

struct deriv_info {
    int is_valid;
    cmpl_vector *val;
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
        struct disp_fb fb;
    } disp;
};

typedef struct disp_struct disp_t;

extern disp_t * load_nk_table(const char * filename, str_ptr *error_msg);
extern disp_t * load_mat_dispers(const char * filename, str_ptr *error_msg);
extern cmpl     n_value(const disp_t *mat, double lam);
extern void     n_value_cpp(const disp_t *mat, double lam,
                            double *nr, double *ni);
extern void     n_value_deriv(const disp_t *disp, cmpl_vector *der,
                              double lambda);
extern double * disp_map_param(disp_t *d, int index);
extern int      dispers_apply_param(disp_t *d, const fit_param_t *fp,
                                    double val);
extern void     get_model_param_deriv(const disp_t *d,
                                      struct deriv_info *der,
                                      const fit_param_t *fp, double lambda,
                                      double *dnr, double *dni);
extern void     disp_free(disp_t *d);
extern disp_t * disp_copy(const disp_t *d);
extern disp_t * disp_new(enum disp_type tp);
extern disp_t * disp_new_with_name(enum disp_type tp, const char *name);
extern int      disp_get_number_of_params(const disp_t *d);
extern double   disp_get_param_value(const disp_t *d, const fit_param_t *fp);
extern int      disp_integrity_check(disp_t *d);
extern int      disp_check_fit_param(disp_t *d, fit_param_t *fp);

extern disp_t * disp_base_copy(const disp_t *src);
extern void     disp_base_free(disp_t *d);
extern int      disp_base_fp_number(const disp_t *src);
extern int      disp_is_tabular(const disp_t *d);
extern int      disp_write(writer_t *w, const disp_t *_d);
extern disp_t * disp_read(lexer_t *l);

__END_DECLS

#endif
