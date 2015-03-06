#include "fit_recipe.h"
#include "dispers-library.h"

fit_recipe::fit_recipe()
{
    fit_config_set_default(config);
    stack = (stack_t*) emalloc(sizeof(stack_t));
    stack_init(stack);
    parameters = fit_parameters_new();
    seeds_list = seed_list_new();
}

fit_recipe::fit_recipe(stack_t *s, const fit_config *cfg, fit_parameters *fps, seeds *sl)
: stack(s), parameters(fps), seeds_list(sl)
{
    *config = *cfg;
}

fit_recipe::~fit_recipe()
{
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

int fit_recipe::write(writer_t *w)
{
    stack_write(w, stack);
    fit_config_write(w, config);
    fit_parameters_write(w, parameters);
    seed_list_write(w, seeds_list);
    return 0;
}

    fit_config config[1];
    stack_t *stack;
    fit_parameters *parameters;
    seeds *seeds_list;

fit_recipe *fit_recipe::read(lexer_t *l)
{
    fit_config config[1];
    fit_parameters *parameters;
    seeds *seeds_list;
    stack_t *stack = stack_read(l);
    if (!stack) return NULL;
    if (fit_config_read(l, config)) goto stack_fail;
    parameters = fit_parameters_read(l);
    if (!parameters) goto stack_fail;
    seeds_list = seed_list_read(l);
    if (!seeds_list) goto params_fail;
    return new fit_recipe(stack, config, parameters, seeds_list);
params_fail:
    fit_parameters_free(parameters);
stack_fail:
    stack_free(stack);
    return NULL;
}
