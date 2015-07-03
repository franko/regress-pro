#ifndef FIT_RECIPE_H
#define FIT_RECIPE_H

#include "fit-engine.h"
#include "multi_sample_recipe.h"
#include "writer.h"
#include "lexer.h"

struct fit_recipe {
    fit_recipe();
    fit_recipe(stack_t *s, const fit_config *cfg, fit_parameters *fps, seeds *sl, multi_sample_recipe *mss);
    ~fit_recipe();

    void setup_default_stack();
    void shift_fit_parameters(const shift_info *shift);
    int write(writer_t *w);
    static fit_recipe *read(lexer_t *l);

    fit_config config[1];
    stack_t *stack;
    fit_parameters *parameters;
    seeds *seeds_list;
    multi_sample_recipe *ms_setup;
};

#endif
