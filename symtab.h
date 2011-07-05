/*
  $Id: symtab.h,v 1.6 2006/12/26 18:15:51 francesco Exp $
 */

#ifndef SYMTAB_H
#define SYMTAB_H

#include "defs.h"
#include "str.h"
#include "stack.h"
#include "dispers.h"
#include "descr-util.h"
#include "fit-params.h"
#include "fit-engine.h"
#include "sample-info.h"

__BEGIN_DECLS

enum toplevel_type {
  TL_TYPE_DISP = 0,
  TL_TYPE_STACK,
  TL_TYPE_STRATEGY,
  TL_TYPE_SAMPLE,
  TL_TYPE_MULTI_FIT,
  TL_TYPE_EXCEED_VALUE
};

struct toplevel_obj {
  enum toplevel_type type;
  union {
    void *generic;
    disp_t *disp;
    struct stack *stack;
    struct strategy *strategy;
    struct sample_info *sample_info;
    struct multi_fit_info *multi_fit;
  } cont;
};

typedef struct toplevel_obj obj_t;

struct assign {
  str_t name;
  obj_t *value;
  struct assign *next;
};

struct use_directives {
  str_t stack;
  str_t strategy;
  str_t multi_fit;
};

struct environment_info {
  str_t script_dir;
};

struct symtab {
  struct assign *assign_start;
  struct use_directives directives[1];
  struct fit_config config_table[1];
  struct environment_info env[1];
};

extern void    symbol_table_clean       (struct symtab *symtab);
extern void    symbol_table_add         (struct symtab *symtab,
					 str_ptr name, obj_t * val);
extern obj_t * symbol_table_find        (struct symtab *symtab, str_ptr name);

extern struct assign * symbol_table_next (struct symtab *symtab,
					  struct assign *iter);

extern obj_t * toplevel_obj_new         (enum toplevel_type kind, void *cont);
extern obj_t * toplevel_obj_copy        (obj_t *obj);
extern void    toplevel_obj_free        (obj_t *obj);

extern int     setup_use_directive      (struct symtab *symtab, 
					 lex_string_t *id, lex_string_t *val);
extern int     set_config_value         (struct symtab *symtab,
					 lex_string_t *id, val_t *val);

extern int     parse_strategy           (struct symtab *symtab, 
					 const char *text);

extern void *  retrieve_parsed_object   (struct symtab *symtab,
					 enum toplevel_type tp, str_ptr name);
extern void    symbol_table_init        (struct symtab symtab[]);
extern void    symbol_table_free        (struct symtab symtab[]);


__END_DECLS

#endif
