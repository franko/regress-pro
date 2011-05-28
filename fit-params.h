#ifndef FIT_PARAMS_H
#define FIT_PARAMS_H

#include "common.h"
#include "spectra.h"
#include "str.h"

__BEGIN_DECLS

enum params_id {
  PID_THICKNESS = 1,
  PID_FIRSTMUL,
  PID_LAYER_N
};

enum disp_model_id {
  MODEL_HO = 1,
  MODEL_CAUCHY,
  MODEL_LOOKUP,
  MODEL_BRUGGEMAN,
  MODEL_NONE,
};

enum seed_type_id {
  SEED_SIMPLE,
  SEED_RANGE,
};

typedef struct {
  enum seed_type_id type;
  double seed;
  double min, max, step;
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

extern struct strategy * strategy_new   (void);
extern void              strategy_free  (struct strategy *s);

extern struct fit_parameters * \
                fit_parameters_new      (void);
extern void     fit_parameters_free     (struct fit_parameters *s);
extern void     fit_parameters_add      (struct fit_parameters *lst,
					 fit_param_t const * fp);

extern int      fit_parameters_are_RI_fixed (struct fit_parameters *f);

extern struct seeds * seed_list_new        (void);
extern void           seed_list_free       (struct seeds *s);
extern void           seed_list_add_simple (struct seeds *s, double v);
extern void           seed_list_add        (struct seeds *s, seed_t *seed);

extern void     set_model_param         (fit_param_t *fpres, int lyr,
					 enum disp_model_id model_id,
					 int param_nb);
extern void     set_thick_param         (fit_param_t *fp, int lyr);
extern void     get_param_name          (fit_param_t *fp, str_t name);
extern int      parse_fit_string        (const char *s, seed_t *seed);

__END_DECLS

#endif
