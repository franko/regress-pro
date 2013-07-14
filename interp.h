#ifndef INTERP_H
#define INTERP_H

#include "str.h"
#include "descr-util.h"
#include "symtab.h"

struct toplevel_parser {
    const char * id;
    obj_t * (*func)(struct symtab *symtab, struct gen_record *);
};

extern str_ptr     get_named_field_as_string(const char *name,
        struct pair_lst *lst);

extern val_lst *   get_named_field_as_list(const char *name,
        struct pair_lst *lst);

extern int         get_named_field_as_double(const char *name,
        struct pair_lst *lst, double *x);

extern struct value * \
get_named_field(const char *name,
                struct pair_lst *lst);

extern int         valuelst_elements_nb(val_lst *lst);

extern obj_t *     toplevel_eval_value(struct symtab *symtab, val_t *v);

extern int         parse_assignment(struct symtab *symtab,
                                    struct pair * p);

#endif
