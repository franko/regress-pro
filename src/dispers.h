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
#include "disp-sellmeier.h"
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

    /* Specific methods for tabular dispersions. */
    int (*samples_number)(const struct disp_struct *d);
    double (*sample_wavelength)(const struct disp_struct *d, int index);

    /* class methods */
    void (*encode_param)(str_t param, const fit_param_t *fp);
    int (*read)(lexer_t *l, struct disp_struct *d);
};

struct deriv_info {
    int is_valid;
    cmpl_vector *val;
};

struct disp_info {
    str_t name;
    str_t description;
    str_ptr modifications_stamp;
    float wavelength_start, wavelength_end;
};

struct disp_struct {
    struct disp_class *dclass;
    enum disp_type type;
    struct disp_info *info;
    union {
        struct disp_table table;
        struct disp_sample_table sample_table;
        struct disp_cauchy cauchy;
        struct disp_sellmeier sellmeier;
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
extern void     disp_set_name(disp_t *d, const char *name);
extern const char *
                disp_get_name(const disp_t *d);
extern int      disp_integrity_check(disp_t *d);
extern int      disp_check_fit_param(disp_t *d, fit_param_t *fp);
extern void     disp_set_info_wavelength(disp_t *d, double wavelength_start, double wavelength_end);
extern void     disp_get_wavelength_range(const disp_t *d, double *wavelength_start, double *wavelength_end, int *samples_number);
extern disp_t * disp_base_copy(const disp_t *src);
extern void     disp_base_free(disp_t *d);
extern int      disp_base_fp_number(const disp_t *src);
extern int      disp_base_write(writer_t *w, const char *tag, const disp_t *d);
extern int      disp_is_tabular(const disp_t *d);
extern int      disp_samples_number(const disp_t *d);
extern double   disp_sample_wavelength(const disp_t *d, int index);
extern void     disp_set_modifications_flag(const disp_t *d, const char *text);
extern int      disp_write(writer_t *w, const disp_t *_d);
extern disp_t * disp_read(lexer_t *l);
extern void     disp_info_copy(struct disp_info *src, struct disp_info *dst);;

// Validity condition for a wavelength range store in disp_info.
#define DISP_VALID_RANGE(wl1, wl2) ((wl1) > 0.0 && (wl2) > (wl1))

__END_DECLS

#endif
