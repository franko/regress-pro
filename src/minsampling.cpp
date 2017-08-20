
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "minsampling.h"
#include "data-view.h"


struct step_data {
    struct data_view *table;
    float epsilon;
    int size;
    int idx_max;
};


static int * minsampling_stepper(struct step_data *info, int idx, int idx_red, int channel_s, int channel_e);

static float tget(const struct step_data *info, int row, int col) {
    return data_view_get(info->table, row, col);
}

int *
minsampling_stepper(struct step_data *info, int idx, int idx_red, int channel_s, int channel_e)
{
    int *map = nullptr;

    const int channels_no = (channel_e - channel_s);
    for (int j = idx + 2; j < info->idx_max; j++) {
        float p[channels_no];
        for (int q = 0; q < channels_no; q++) {
            const int ch = channel_s + q;
            p[q] = (tget(info, j, ch) - tget(info, idx, ch)) / (tget(info, j, 0) - tget(info, idx, 0));
        }

        for (int k = idx+1; k < j; k++) {
            for (int q = 0; q < channels_no; q++) {
                const int ch = channel_s + q;
                const float yint = tget(info, idx, ch) + (tget(info, k, 0) - tget(info, idx, 0)) * p[q];
                const float del = yint - tget(info, k, ch);
                if (fabsf(del) > info->epsilon) {
                    map = minsampling_stepper(info, j - 1, idx_red + 1, channel_s, channel_e);
                    /* map cannot be null here */
                    map[idx_red] = j - 1;
                    return map;
                }
            }
        }
    }

    info->size = idx_red + 1;

    map = (int *) emalloc((idx_red + 1) * sizeof(int));
    map[idx_red] = info->idx_max - 1;
    return map;
}

void
table_sample_minimize(struct spectrum *s, float dlmt, int channel_s, int channel_e)
{
    struct step_data info[1];
    int *map;

    info->idx_max = s->table->rows;
    info->table   = s->table;
    info->epsilon = dlmt;

    map = minsampling_stepper(info, s->table->idx_start, 1, channel_s, channel_e);
    map[0] = 0;

#ifdef DEBUG
    fprintf(stderr, "number of channels: %d, original size: %d, reduced size: %d\n", channel_e - channel_s, info->idx_max, info->size);
    for (int j = 0; j < info->size; j++) {
        fprintf(stderr, " %d", map[j]);
    }
    fprintf(stderr, "\n");
#endif

    data_view_set_map(s->table, info->size, map);
}
