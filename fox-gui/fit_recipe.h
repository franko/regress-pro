#ifndef FIT_RECIPE_H
#define FIT_RECIPE_H

#include "fit-engine.h"

struct fit_recipe {
    fit_recipe();
    ~fit_recipe();

    void setup_default_stack();

    fit_config config[1];
    stack_t *stack;
    fit_parameters *parameters;
    seeds *seeds_list;
    fit_engine *fit;
};

#endif
