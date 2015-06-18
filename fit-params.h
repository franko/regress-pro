#ifndef FIT_PARAMS_H
#define FIT_PARAMS_H

#include "common.h"
#include "spectra.h"
#include "str.h"
#include "writer.h"

__BEGIN_DECLS

enum params_id {
    PID_THICKNESS = 1,
    PID_LAYER_N,
    PID_LAYER_INDIPENDENT, /* Beginning of layer-indipendent parameters */
    PID_FIRSTMUL = PID_LAYER_INDIPENDENT,
    PID_INVALID,
};

/* Used to inform about the insert or delete of a layer in a
   film stack. */
enum {
    SHIFT_DELETE_LAYER,
    SHIFT_INSERT_LAYER,
};

struct shift_info {
    short int event; /* Should be on the SHIFT_* enumerated values. */
    short int index; /* The index of the layer removed on inserted. */
};

enum disp_model_id {
    MODEL_HO = 1,
    MODEL_CAUCHY,
    MODEL_LOOKUP,
    MODEL_BRUGGEMAN,
    MODEL_FB,
    MODEL_NONE,
};

enum seed_type_id {
    SEED_SIMPLE,
    SEED_RANGE,
    SEED_UNDEF,
};

typedef struct {
    enum seed_type_id type;
    double seed;
    double delta;
} seed_t;

typedef struct {
    enum params_id id;
    int layer_nb;
    enum disp_model_id model_id;
    int param_nb;
} fit_param_t;

struct fit_parameters {
    size_t number;
    size_t alloc;
    fit_param_t *values;
};

struct seeds {
    size_t number;
    size_t alloc;
    seed_t *values;
};

struct strategy {
    struct fit_parameters *parameters;
    struct seeds *seeds;
};

extern struct strategy * strategy_new(void);
extern void              strategy_free(struct strategy *s);

extern struct fit_parameters * \
fit_parameters_new(void);
extern void     fit_parameters_free(struct fit_parameters *s);
extern struct fit_parameters *fit_parameters_copy(const struct fit_parameters *fps);
extern void     fit_parameters_clear(struct fit_parameters *s);
extern int      fit_parameters_add(struct fit_parameters *lst,
                                   fit_param_t const * fp);
extern void     fit_parameters_remove(struct fit_parameters *lst, int index);
extern void     fit_parameters_fix_layer_shift(struct fit_parameters *lst, struct shift_info shift);
extern int      fit_parameters_find(const struct fit_parameters *lst, const fit_param_t *fp);

extern int      fit_parameters_are_RI_fixed(struct fit_parameters *f);
extern int      fit_param_compare(const fit_param_t *a, const fit_param_t *b);
extern int      fit_parameters_write(writer_t *w, const struct fit_parameters *s);
extern struct fit_parameters *fit_parameters_read(lexer_t *l);

extern struct seeds * seed_list_new(void);
extern void           seed_list_free(struct seeds *s);
extern void           seed_list_add_simple(struct seeds *s, double v);
extern void           seed_list_add(struct seeds *s, const seed_t *seed);
extern void           seed_list_remove(struct seeds *lst, int index);
extern int            seed_list_write(writer_t *w, const struct seeds *s);
extern struct seeds * seed_list_read(lexer_t *l);

extern void     set_model_param(fit_param_t *fpres, int lyr,
                                enum disp_model_id model_id,
                                int param_nb);
extern void     set_thick_param(fit_param_t *fp, int lyr);
extern void     get_param_name(const fit_param_t *fp, str_t name);
extern void     get_full_param_name(const fit_param_t *fp, str_t name);

__END_DECLS

#endif
