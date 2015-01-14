#ifndef DISPERS_LIBRARY_H
#define DISPERS_LIBRARY_H

#include "defs.h"
#include "dispers.h"

__BEGIN_DECLS

struct disp_node {
    disp_t *content;
    struct disp_node *next;
};

struct disp_list {
    struct disp_node *first;
    struct disp_node *last;
};

extern void     dispers_library_init(void);
extern disp_t * dispers_library_search(const char *id);
extern disp_t * dispers_library_get(int index, char const ** lname);

extern struct disp_node * disp_list_add(struct disp_list *lst, disp_t *d);
extern void               disp_list_remove(struct disp_list *lst, struct disp_node *prev);
extern void               disp_list_free(struct disp_list *lst);

extern struct disp_list user_lib[1];

__END_DECLS

#endif
