
/*
  $Id$
*/

#include <string.h>

#include "sample-info.h"

struct sample_info *
sample_info_new(int constr_nb, int indiv_nb) {
    struct sample_info *r;
    r = emalloc(sizeof(struct sample_info));
    str_init(r->spectrum_name, 64);

    if(constr_nb > 0) {
        r->constraints = emalloc(constr_nb * sizeof(double));
    } else {
        r->constraints = NULL;
    }
    r->constr_nb = constr_nb;

    if(indiv_nb > 0) {
        r->individual  = emalloc(indiv_nb  * sizeof(double));
    } else {
        r->individual = NULL;
    }
    r->indiv_nb = indiv_nb;

    return r;
}

struct sample_info *
sample_info_copy(struct sample_info const *src) {
    struct sample_info *r;

    r = emalloc(sizeof(struct sample_info));
    str_init_from_str(r->spectrum_name, src->spectrum_name);

    if(src->constraints) {
        r->constraints = emalloc(src->constr_nb * sizeof(double));
        memcpy(r->constraints, src->constraints, src->constr_nb * sizeof(double));
    } else {
        r->constraints = NULL;
    }
    r->constr_nb = src->constr_nb;

    if(src->individual) {
        r->individual  = emalloc(src->indiv_nb  * sizeof(double));
        memcpy(r->individual, src->individual, src->indiv_nb * sizeof(double));
    } else {
        r->individual = NULL;
    }
    r->indiv_nb = src->indiv_nb;

    return r;
}

void
sample_info_free(struct sample_info *r)
{
    if(r->constraints) {
        free(r->constraints);
    }
    if(r->individual) {
        free(r->individual);
    }
    str_free(r->spectrum_name);
    free(r);
}

struct multi_fit_info *
multi_fit_info_new(int samples_number) {
    struct multi_fit_info *r;
    int k;

    r = emalloc(sizeof(struct multi_fit_info));

    r->samples_number = samples_number;

    r->constraints.parameters = fit_parameters_new();
    r->constraints.seeds      = seed_list_new();

    r->individual.parameters = fit_parameters_new();
    r->individual.seeds      = seed_list_new();

    r->spectra_list = emalloc(samples_number * sizeof(struct spectrum *));

    for(k = 0; k < samples_number; k++) {
        r->spectra_list[k] = NULL;
    }

    return r;
}

void
multi_fit_info_free(struct multi_fit_info *inf)
{
    int k;

    for(k = 0; k < inf->samples_number; k++) {
        if(inf->spectra_list[k] != NULL) {
            spectra_free(inf->spectra_list[k]);
        }
    }
    free(inf->spectra_list);

    fit_parameters_free(inf->constraints.parameters);
    fit_parameters_free(inf->individual.parameters);

    seed_list_free(inf->constraints.seeds);
    seed_list_free(inf->individual.seeds);

    free(inf);
}
