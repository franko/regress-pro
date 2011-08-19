#ifndef DESCR_UTIL_H
#define DESCR_UTIL_H

#include <stdio.h>
#include "common.h"
#include "str.h"

enum value_tp {
  VAL_TP_DOUBLE,
  VAL_TP_STRING,
  VAL_TP_RECORD,
  VAL_TP_LIST,
  VAL_TP_VARREF
};

struct value {
  enum value_tp type;
  union {
    str_ptr string;
    double real;
    struct gen_record * record;
    struct value_lst * vallist;
  } cont;
};

struct gen_record {
  str_ptr id;
  int singleton;
  union {
    struct value * value;
    struct pair_lst * list;
  } cont;
};

struct value_lst {
  struct value * value;
  struct value_lst *next;
};

struct pair {
  str_ptr key;
  struct value * value;
};

struct pair_lst {
  struct pair * field;
  struct pair_lst *next;
};

typedef struct {
  int index;
  int length;
} lex_string_t;

typedef struct value val_t;
typedef struct value_lst val_lst;

extern val_t *          value_from_string       (lex_string_t *s,
						 enum value_tp tp);
extern val_t *          value_from_double       (double x);
extern val_t *          value_from_list         (val_lst *lst);
extern void             value_free              (struct value *v);
extern void             value_list_free         (struct value_lst *lst);
extern struct pair *    build_pair              (lex_string_t * fieldname,
						 val_t * val);
extern val_lst *        list_add_value          (val_t *v, val_lst *lst);
extern struct pair_lst *pairlist_add_pair       (struct pair *p,
						 struct pair_lst *lst);
extern val_t *          build_rec_from_value    (lex_string_t *tag,
						 val_t *val);
extern val_t *          build_rec_from_pairlist (lex_string_t *tag,
						 struct pair_lst *plst);
extern void             pair_free               (struct pair *p);
extern void             pair_list_free          (struct pair_lst *plst);

#endif
