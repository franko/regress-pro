#include "fit_recipe.h"
#include "dispers-library.h"

fit_recipe::fit_recipe()
{
    fit_config_set_default(config);
    stack = (stack_t*) emalloc(sizeof(stack_t));
    stack_init(stack);
    parameters = fit_parameters_new();
    seeds_list = seed_list_new();
    fit = fit_engine_new();
}

fit_recipe::~fit_recipe()
{
    fit_engine_free(fit);
    seed_list_free(seeds_list);
    fit_parameters_free(parameters);
    stack_free(stack);
}


void fit_recipe::setup_default_stack()
{
    disp_t *si = dispers_library_search("si");
    disp_t *sio2 = dispers_library_search("sio2");
    disp_t *vac = dispers_library_search("vacuum");
    stack_add_layer(stack, vac, 0.0);
    stack_add_layer(stack, sio2, 10.0);
    stack_add_layer(stack, si, 0.0);
}

void fit_recipe::shift_fit_parameters(const shift_info *shift)
{
    fit_parameters_fix_layer_shift(parameters, *shift);
}
