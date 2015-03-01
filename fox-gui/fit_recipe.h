#ifndef FIT_RECIPE_H
#define FIT_RECIPE_H

#include "fit-engine.h"
#include "writer.h"

struct fit_recipe {
    fit_recipe();
    ~fit_recipe();

    void setup_default_stack();
    void shift_fit_parameters(const shift_info *shift);
    int write(writer_t *w);

    fit_config config[1];
    stack_t *stack;
    fit_parameters *parameters;
    seeds *seeds_list;
};

#endif
