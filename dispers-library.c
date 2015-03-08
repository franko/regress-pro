#include <string.h>

#include "common.h"
#include "data-table.h"
#include "dispers.h"
#include "dispers-library.h"

struct disp_list_node {
    disp_t *content;
    const char *id;
    struct disp_list_node *next;
};

/* this structure mirror the struct data_table in data-table.h
   with the difference that a bigger array of floats is statically
   allocated. */
struct data_table_silicon {
    int rows;
    int columns;
    int ref_count;
    float heap[341 * 2];
};

static void add_dispersion_node(struct disp_list_node *curr,
                                struct disp_list_node *prev,
                                const char *id, disp_t *disp);

#include "si-paper-table.h"
#include "poly-table.h"

static struct disp_list_node *dispers_library_list;

struct disp_list user_lib[1] = {{NULL, NULL}};

/* Register a table-based dispersion with an unique id. */
struct disp_table_entry {
    const char *id;
    struct disp_table *disp_table;
};

static struct disp_table_entry lib_disp_table[6];
const int lib_poly_index = 0;
const int lib_si_index = 5;
const int lib_end_index = 6;

void
add_dispersion_node(struct disp_list_node *curr, struct disp_list_node *prev,
                    const char *id, disp_t *disp)
{
    curr->content = disp;
    curr->id = id;
    curr->next = prev;
}

static disp_t *
make_poly_dispers()
{
    static const char *comp_name[5] = {"siamoimp-1", "siamo-1", "poly-1", "poly-as-1", "si-w-1"};
    disp_t *lookup_disp;
    const int nb_comp = 5;
    struct lookup_comp *comp_data = malloc(nb_comp * sizeof(struct lookup_comp));
    int j = 0;

    struct disp_table_entry *lib_entry = &lib_disp_table[lib_poly_index];
    for(j = 0; j < nb_comp; j++, lib_entry++) {
        struct lookup_comp *comp = comp_data + j;
        struct disp_table *dt;

        comp->p = poly_p_value[j];

        comp->disp = disp_new_with_name(DISP_TABLE, poly_comp_name[j]);
        dt = &comp->disp->disp.table;

        dt->points_number = 271;
        dt->lambda_min    = 240.0;
        dt->lambda_max    = 780.0;
        dt->lambda_stride = 2.0;
        dt->table_ref     = (struct data_table *) &poly_comp[j];

        lib_entry->id = comp_name[j];
        lib_entry->disp_table = dt;
    }

    lookup_disp = disp_new_lookup("Poly", 5, comp_data, 0.5);

    return lookup_disp;
}

void
dispers_library_init()
{
#define NB_LIBRARY_DISPERS 5
    static struct disp_list_node node_prealloc[NB_LIBRARY_DISPERS];
    static struct ho_params sio2_ho_params[] = {
        {145.0, 15.78839, 0.0, 0.3333, 0.0}
    };
    struct disp_table *dt;
    double vac_cauchy_n[3] = {1, 0, 0};
    double vac_cauchy_k[3] = {0, 0, 0};
    double water_cauchy_n[3] = {1.31970012187958, 4677.3115234375, -108020320};
    double *water_cauchy_k = vac_cauchy_k;
    struct disp_list_node *node, *prev;
    disp_t *current;
    int idx = 0;

    prev = NULL;

    /* Silicon PAPER */
    current = disp_new_with_name(DISP_TABLE, "Si from paper");
    dt = & current->disp.table;

    dt->points_number = si_data_table.rows;
    dt->lambda_min    = SI_PAPER_WVLEN_MIN;
    dt->lambda_max    = SI_PAPER_WVLEN_MAX;
    dt->lambda_stride = SI_PAPER_WVLEN_STRIDE;
    dt->table_ref     = (struct data_table *) & si_data_table;

    lib_disp_table[lib_si_index].id = "si-paper-1";
    lib_disp_table[lib_si_index].disp_table = dt;

    node = node_prealloc + idx;
    add_dispersion_node(node, prev, "si", current);
    prev = node;
    idx ++;

    /* Vacuum dispersion */
    current = disp_new_cauchy("vacuum", vac_cauchy_n, vac_cauchy_k);

    node = node_prealloc + idx;
    add_dispersion_node(node, prev, "vacuum", current);
    prev = node;
    idx ++;

    /* water dispersion */
    current = disp_new_cauchy("water", water_cauchy_n, water_cauchy_k);

    node = node_prealloc + idx;
    add_dispersion_node(node, prev, "water", current);
    prev = node;
    idx ++;

    current = disp_new_ho("Thermal SiO2", 1, sio2_ho_params);

    node = node_prealloc + idx;
    add_dispersion_node(node, prev, "sio2", current);
    prev = node;
    idx ++;

    current = make_poly_dispers();

    node = node_prealloc + idx;
    add_dispersion_node(node, prev, "poly", current);
    prev = node;
    idx ++;

    dispers_library_list = node;
}

disp_t *
dispers_library_search(const char *id)
{
    struct disp_list_node *node;
    disp_t *result = NULL;

    for(node = dispers_library_list; node; node = node->next) {
        if(strcasecmp(node->id, id) == 0) {
            result = disp_copy(node->content);
            break;
        }
    }

    return result;
}

disp_t *
dispers_library_get(int index, char const ** lname)
{
    struct disp_list_node *n;
    for(n = dispers_library_list; n; n = n->next, index--) {
        if(index == 0) {
            *lname = n->id;
            return n->content;
        }
    }
    return NULL;
}

static struct disp_node *new_disp_node(disp_t *d)
{
    struct disp_node *n = malloc(sizeof(struct disp_node));
    n->content = d;
    n->next = NULL;
    return n;
}

static void free_disp_node(struct disp_node *n)
{
    disp_free(n->content);
    free(n);
}

struct disp_node *
disp_list_add(struct disp_list *lst, disp_t *d)
{
    struct disp_node *n = new_disp_node(d);
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

const char *
lib_disp_table_lookup(const disp_t *d)
{
    if (d->type != DISP_TABLE) return NULL;
    const struct disp_table *dt = &d->disp.table;
    struct disp_table_entry *p = lib_disp_table;
    int i;
    for (i = 0; i < lib_end_index; i++, p++) {
        if (p->disp_table->table_ref == dt->table_ref) {
            return p->id;
        }
    }
    return NULL;
}

disp_t *
lib_disp_table_get(const char *id)
{
    struct disp_table_entry *p = lib_disp_table;
    int i;
    for (i = 0; i < lib_end_index; i++, p++) {
        if (strcmp(p->id, id) == 0) {
            disp_t *d = disp_new_with_name(DISP_TABLE, p->id);
            d->disp.table = *p->disp_table;
            return d;
        }
    }
    return NULL;
}
