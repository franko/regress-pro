#ifndef DISP_FB_PRIV_H
#define DISP_FB_PRIV_H

#include "dispers.h"

extern void     fb_free(disp_t *d);
extern disp_t * fb_copy(const disp_t *d);

extern cmpl fb_n_value(const disp_t *disp, double lam);
extern cmpl fb_n_value_deriv(const disp_t *disp, double lam,
                             cmpl_vector *der);
extern int  fb_fp_number(const disp_t *disp);
extern double * fb_map_param(disp_t *disp, int index);
extern int  fb_apply_param(struct disp_struct *d,
                           const fit_param_t *fp, double val);
extern void fb_encode_param(str_t param, const fit_param_t *fp);

extern double fb_get_param_value(const struct disp_struct *d,
                                 const fit_param_t *fp);

extern int fb_write(writer_t *w, const disp_t *_d);
extern int fb_read(lexer_t *l, disp_t *d);

extern void tauc_lorentz_change_form(struct disp_fb *fb, int new_coeff_form);

#endif
