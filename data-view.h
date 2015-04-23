#ifndef DATA_VIEW_H
#define DATA_VIEW_H

#include "data-table.h"

__BEGIN_DECLS

#define DATA_VIEW_IDX(d,r,c) (d->map ? (d)->table->columns * ((d)->idx_start + (d)->map[r]) + (c) : (d)->table->columns * ((d)->idx_start + (r)) + (c))
#define data_view_get(d,r,c) (d)->table->heap[DATA_VIEW_IDX(d,r,c)]
#define data_view_get_row(d,r) ((d)->table->heap + DATA_VIEW_IDX(d,r,0))

struct data_view {
    /* a data view can have a number of rows smaller that the table */
    int rows;
    int columns; /* the number of columns will be always the same */

    int idx_start;
    int idx_end;

    int *map;

    struct data_table *table;
};

void data_view_init(struct data_view *d, struct data_table *table);

void data_view_init_empty(struct data_view *d);

void data_view_copy(struct data_view *dest,
                    const struct data_view *src);

void data_view_dealloc(struct data_view *d);

void data_view_set_mask_offset(struct data_view *d, int start, int end);

void data_view_set_map(struct data_view *d, int size, int *map);

void data_view_reset(struct data_view *d);

__END_DECLS

#endif
