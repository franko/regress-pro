/*
  $Id: stack.h,v 1.6 2006/07/12 22:48:55 francesco Exp $
 */

#ifndef STACK_H
#define STACK_H

#include "cmpl.h"
#include "dispers.h"

__BEGIN_DECLS

struct stack {
  int nb; /* number of mediums */
  struct disp_struct ** disp;
  double *thickness;
  size_t nb_alloc;
};

typedef struct stack stack_t;

extern int      stack_apply_param       (stack_t *s, const fit_param_t *fp,
					 double val);
extern void     stack_free              (stack_t *d);
extern stack_t *stack_copy              (stack_t *s);
extern void     stack_init              (stack_t *s);
extern void     stack_init_raw          (stack_t *s, size_t nb_init);
extern void     stack_add_layer         (stack_t *s, disp_t *lyr, double th);
extern const 
double *        stack_get_ths_list      (const stack_t *s);
extern void     stack_get_ns_list       (stack_t *s, cmpl *ns, double lambda);

__END_DECLS

#endif
