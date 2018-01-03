#ifndef FIT_PARAMS_H
#define FIT_PARAMS_H

#include "generic_pod_vector.h"
#include "common.h"
#include "str.h"
#include "writer.h"
#include "lexer.h"
#include "dispers-classes.h"

enum params_id {
    PID_THICKNESS = 1,
    PID_LAYER_N,
    PID_ACQUISITION_PARAMETER, /* Beginning of acquisition parameters */
    PID_FIRSTMUL = PID_ACQUISITION_PARAMETER,
    PID_AOI,
    PID_ANALYZER,
    PID_POLARIZER,
    PID_BANDWIDTH,
    PID_NUMAP,
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
    enum disp_type model_id;
    int param_nb;
} fit_param_t;

class fit_parameters : public pod_vector_base<fit_param_t> {
    using base_type = pod_vector_base<fit_param_t>;
    using base_type::base_type;
};

class seeds_list : public pod_vector_base<seed_t> {
    using base_type = pod_vector_base<seed_t>;
    using base_type::base_type;
};

struct strategy {
    fit_parameters *parameters;
    seeds_list *seeds;
};

extern struct strategy * strategy_new(void);
extern void              strategy_free(struct strategy *s);

extern fit_parameters * fit_parameters_new();
extern void     fit_parameters_free(fit_parameters *s);
extern fit_parameters *fit_parameters_copy(const fit_parameters *fps);
extern void     fit_parameters_clear(fit_parameters *s);
extern int      fit_parameters_add(fit_parameters *lst,
                                   fit_param_t const * fp);
extern void     fit_parameters_remove(fit_parameters *lst, int index);
extern void     fit_parameters_fix_layer_shift(fit_parameters *lst, struct shift_info shift);
extern int      fit_parameters_find(const fit_parameters *lst, const fit_param_t *fp);

extern int      fit_parameters_are_RI_fixed(fit_parameters *f);
extern int      fit_parameters_contains_acquisition_parameters(fit_parameters *f);
extern int      fit_param_compare(const fit_param_t *a, const fit_param_t *b);
extern int      fit_parameters_write(writer_t *w, const fit_parameters *s);
extern fit_parameters *fit_parameters_read(lexer_t *l);

extern seeds_list * seed_list_new();
extern void           seed_list_free(seeds_list *s);
extern void           seed_list_add_simple(seeds_list *s, double v);
extern void           seed_list_add(seeds_list *s, const seed_t *seed);
extern void           seed_list_remove(seeds_list *lst, int index);
extern int            seed_list_write(writer_t *w, const seeds_list *s);
extern seeds_list * seed_list_read(lexer_t *l);

extern void     set_model_param(fit_param_t *fpres, int lyr,
                                enum disp_type model_id,
                                int param_nb);
extern void     set_thick_param(fit_param_t *fp, int lyr);
extern void     get_param_name(const fit_param_t *fp, str_t name);
extern void     get_full_param_name(const fit_param_t *fp, str_t name);

#endif
