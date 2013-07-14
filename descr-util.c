#include "descr-util.h"
#include "fitlexer.h"

static void gen_record_free(struct gen_record *r);

struct value *
value_from_string(lex_string_t *s, enum value_tp tp) {
    struct value *val = emalloc(sizeof(struct value));
    val->type = tp;
    val->cont.string = get_lex_string(s);
    return val;
}

struct pair *
build_pair(lex_string_t * fieldname, struct value * val) {
    struct pair *res = emalloc(sizeof(struct pair));
    res->key = get_lex_string(fieldname);
    res->value = val;
    return res;
}

struct value_lst *
list_add_value(struct value *v, struct value_lst *lst) {
    struct value_lst *res, *top = lst;

    while(lst) {
        if(lst->next == NULL) {
            break;
        }
        lst = lst->next;
    }

    res = emalloc(sizeof(struct value_lst));
    res->value = v;
    res->next = NULL;

    if(lst) {
        lst->next = res;
        return top;
    }

    return res;
}

struct pair_lst *
pairlist_add_pair(struct pair *p, struct pair_lst *lst) {
    struct pair_lst *res = emalloc(sizeof(struct pair_lst));
    res->field = p;
    res->next = lst;
    return res;
}

struct value *
value_from_double(double x) {
    struct value *val = emalloc(sizeof(struct value));
    val->type = VAL_TP_DOUBLE;
    val->cont.real = x;
    return val;
}

struct value *
value_from_list(struct value_lst *lst) {
    struct value *val = emalloc(sizeof(struct value));
    val->type = VAL_TP_LIST;
    val->cont.vallist = lst;
    return val;
}

struct value *
build_rec_from_pairlist(lex_string_t *tag, struct pair_lst *plst) {
    struct gen_record *rec = emalloc(sizeof(struct gen_record));
    struct value *val = emalloc(sizeof(struct value));

    rec->id = get_lex_string(tag);
    rec->singleton = 0;
    rec->cont.list = plst;

    val->type = VAL_TP_RECORD;
    val->cont.record = rec;

    return val;
}

struct value *
build_rec_from_value(lex_string_t *tag, struct value *val) {
    struct gen_record *rec = emalloc(sizeof(struct gen_record));
    struct value *res = emalloc(sizeof(struct value));

    rec->id = get_lex_string(tag);
    rec->singleton = 1;
    rec->cont.value = val;

    res->type = VAL_TP_RECORD;
    res->cont.record = rec;

    return res;
}

void
value_free(struct value *v)
{
    switch(v->type) {
    case VAL_TP_VARREF:
    case VAL_TP_STRING:
        str_free(v->cont.string);
        free(v->cont.string);
        break;
    case VAL_TP_RECORD:
        gen_record_free(v->cont.record);
        break;
    case VAL_TP_LIST:
        value_list_free(v->cont.vallist);
        break;
    default:
        /* */
        ;
    }
    free(v);
}

void
gen_record_free(struct gen_record *r)
{
    if(r->singleton) {
        value_free(r->cont.value);
    } else {
        pair_list_free(r->cont.list);
    }
    str_free(r->id);
    free(r->id);
    free(r);
}

void
value_list_free(struct value_lst *lst)
{
    while(lst) {
        struct value_lst *next = lst->next;
        value_free(lst->value);
        free(lst);
        lst = next;
    }
}

void
pair_free(struct pair *p)
{
    str_free(p->key);
    free(p->key);
    value_free(p->value);
    free(p);
}

void
pair_list_free(struct pair_lst *plst)
{
    while(plst) {
        struct pair_lst *next = plst->next;
        pair_free(plst->field);
        free(plst);
        plst = next;
    }
}
