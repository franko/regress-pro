#include <string.h>

#include "common.h"
#include "dispers.h"
#include "dispers-library.h"
#include "dispers_library_preload.h"
#include "preset_library_data.h"

struct disp_list app_lib[1] = {{NULL, NULL}};
struct disp_list user_lib[1] = {{NULL, NULL}};
struct disp_list preset_lib[1] = {{NULL, NULL}};

static struct disp_node *new_disp_node(disp_t *d, const char *id)
{
    struct disp_node *n = malloc(sizeof(struct disp_node));
    if (id) {
        n->id = str_new();
        str_copy_c(n->id, id);
    } else {
        n->id = NULL;
    }
    n->content = d;
    n->next = NULL;
    return n;
}

static void free_disp_node(struct disp_node *n)
{
    if (n->id) {
        str_free(n->id);
        free(n->id);
    }
    disp_free(n->content);
    free(n);
}

struct disp_node *
disp_list_add(struct disp_list *lst, disp_t *d, const char *id)
{
    struct disp_node *n = new_disp_node(d, id);
    if (lst->last) {
        lst->last->next = n;
        lst->last = n;
    } else {
        lst->first = n;
        lst->last = n;
    }
    return n;
}

void
disp_list_remove(struct disp_list *lst, struct disp_node *prev)
{
    struct disp_node *n = (prev ? prev->next : lst->first);
    struct disp_node *next = n->next;
    free_disp_node(n);
    if (prev) {
        prev->next = next;
    } else {
        lst->first = next;
    }
    if (next == NULL) {
        lst->last = prev;
    }
}

void
disp_list_free(struct disp_list *lst)
{
    struct disp_node *n, *next;
    for (n = lst->first; n; n = next) {
        next = n->next;
        free_disp_node(n);
    }
    lst->first = NULL;
    lst->last = NULL;
}

disp_t *
disp_list_search(struct disp_list *lst, const char *id)
{
    struct disp_node *n;
    for (n = app_lib->first; n; n = n->next) {
        if (n->id && strcmp(CSTR(n->id), id) == 0) {
            return disp_copy(n->content);
        }
    }
    return NULL;
}

disp_t *
disp_list_get_by_index(struct disp_list *lst, int index)
{
    struct disp_node *n;
    for (n = lst->first; n; n = n->next, index--) {
        if (index == 0) {
            return disp_copy(n->content);
        }
    }
    return NULL;
}

int
disp_list_length(struct disp_list *lst)
{
    int count = 0;
    struct disp_node *n;
    for (n = lst->first; n; n = n->next) {
        count++;
    }
    return count;
}

const char *
lib_disp_table_lookup(const disp_t *d)
{
    struct disp_node *n;
    for (n = app_lib->first; n; n = n->next) {
        if (n->id && n->content->type == d->type) {
            if (d->type == DISP_TABLE && n->content->disp.table.table_ref == d->disp.table.table_ref) {
                return CSTR(n->id);
            }
            if (d->type == DISP_SAMPLE_TABLE && n->content->disp.sample_table.table == d->disp.sample_table.table) {
                return CSTR(n->id);
            }
        }
    }
    return NULL;
}

disp_t *
lib_disp_table_get(const char *id)
{
    struct disp_node *n;
    for (n = app_lib->first; n; n = n->next) {
        if (n->id && strcmp(CSTR(n->id), id) == 0) {
            return disp_copy(n->content);
        }
    }
    return NULL;
}

int
load_library_from_data(struct disp_list *lib, const char *data)
{
    str_t disp_id;
    str_init(disp_id, 15);
    int status = 1;
    int i, n;
    lexer_t *l = lexer_new(data);
    if (lexer_check_ident(l, "dispers-library")) goto init_exit;
    if (lexer_integer(l, &n)) goto init_exit;
    for (i = 0; i < n; i++) {
        if (lexer_check_ident(l, "library-id")) goto init_exit;
        if (lexer_string(l)) goto init_exit;
        str_copy(disp_id, l->store);
        disp_t *d = disp_read(l);
        if (!d) goto init_exit;
        disp_list_add(lib, d, CSTR(disp_id));
    }
    if (l->current.tk != TK_EOF) goto init_exit;
    status = 0;
init_exit:
    str_free(disp_id);
    lexer_free(l);
    return status;
}

int dispers_library_init()
{
    int status;
    status = load_library_from_data(app_lib, dispersions_data);
    if (status != 0) return status;
    status = load_library_from_data(preset_lib, preset_library_data);
    return status;
};
