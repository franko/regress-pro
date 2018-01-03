#ifndef DISPERS_LIBRARY_H
#define DISPERS_LIBRARY_H

#include "defs.h"
#include "dispers.h"
#include "str.h"

struct disp_node {
    disp_t *content;
    str_ptr id;
    struct disp_node *next;
};

struct disp_list {
    struct disp_node *first;
    struct disp_node *last;
};

extern int dispers_library_init();

extern struct disp_node * disp_list_add(struct disp_list *lst, disp_t *d, const char *id);
extern void               disp_list_remove(struct disp_list *lst, struct disp_node *prev);
extern void               disp_list_free(struct disp_list *lst);
extern disp_t *           disp_list_search(struct disp_list *lst, const char *id);
extern disp_t *           disp_list_get_by_index(struct disp_list *lst, int index);
extern int                disp_list_length(struct disp_list *lst);

extern const char *lib_disp_table_lookup(const disp_t *d);
extern disp_t *lib_disp_table_get(const char *id);

extern struct disp_list app_lib[1];
extern struct disp_list user_lib[1];
extern struct disp_list preset_lib[1];

#endif
