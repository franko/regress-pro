#ifndef MULTI_SAMPLE_RECIPE_H
#define MULTI_SAMPLE_RECIPE_H

#include "fit-params.h"
#include "writer.h"
#include "lexer.h"

struct multi_sample_recipe {
    multi_sample_recipe();
    multi_sample_recipe(fit_parameters *ips, fit_parameters *cs): iparameters(ips), cparameters(cs) {}
    ~multi_sample_recipe();

    int write(writer_t *w);
    static multi_sample_recipe *read(lexer_t *l);

    fit_parameters *iparameters;
    fit_parameters *cparameters;
};

#endif
