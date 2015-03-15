#include "multi_sample_recipe.h"

multi_sample_recipe::multi_sample_recipe()
{
    iparameters = fit_parameters_new();
    cparameters = fit_parameters_new();
}

multi_sample_recipe::~multi_sample_recipe()
{
    fit_parameters_free(iparameters);
    fit_parameters_free(cparameters);
}

int multi_sample_recipe::write(writer_t *w)
{
    writer_printf(w, "multi-sample");
    writer_newline_enter(w);
    fit_parameters_write(w, iparameters);
    fit_parameters_write(w, cparameters);
    writer_indent(w, -1);
    return 0;
}

multi_sample_recipe *multi_sample_recipe::read(lexer_t *l)
{
    if (lexer_check_ident(l, "multi-sample")) return NULL;
    fit_parameters *iparameters = fit_parameters_read(l);
    if (!iparameters) return NULL;
    fit_parameters *cparameters = fit_parameters_read(l);
    if (!cparameters) goto ms_read_exit;
    return new multi_sample_recipe(iparameters, cparameters);
ms_read_exit:
    fit_parameters_free(iparameters);
    return NULL;
}
