#include <stdio.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_deriv.h>

#include "cmpl.h"
#include "dispers.h"
#include "fit-params.h"

enum { REAL_PART, IMAG_PART };

struct eval_param {
	disp_t *d;
	double lambda;
	fit_param_t fit_param[1];
	int n_part;
};

double n_eval(double x, void *params)
{
	struct eval_param *p = (struct eval_param *) params;
	dispers_apply_param(p->d, p->fit_param, x);
  	cmpl n = n_value(p->d, p->lambda);
  	return (p->n_part == REAL_PART ? creal(n) : cimag(n));
}

void
test_dispers_deriv(const disp_t *d_src, int model_id, double lambda)
{
	struct eval_param p[1];

	FILE *f = fopen("output.txt", "w");

	p->d = disp_copy(d_src);
	p->lambda = lambda;
	p->fit_param->id = PID_LAYER_N;
	p->fit_param->layer_nb = 0;
	p->fit_param->model_id = model_id;
	p->fit_param->param_nb = -1;
	p->n_part = REAL_PART;

	int no_params = disp_get_number_of_params(p->d);
	cmpl_vector *der_array = cmpl_vector_alloc(no_params);

	n_value_deriv(p->d, der_array, p->lambda);

	gsl_function F;
	F.function = &n_eval;
	F.params = (void *) p;

	int q;
	for (q = 0; q < 2; q++) {
		p->n_part = (q == 0 ? REAL_PART : IMAG_PART);
		int i;
		for (i = 0; i < no_params; i++) {
			p->fit_param->param_nb = i;

			double num_der, abserr;
			double p0 = disp_get_param_value(p->d, p->fit_param);
			double h = p0 * 1e-4;
			h = (h < 1e-6 ? 1e-6 : h);
			gsl_deriv_central(&F, p0, h, &num_der, &abserr);
			dispers_apply_param(p->d, p->fit_param, p0);

			cmpl nmod = cmpl_vector_get(der_array, i);
			double mod_der = (p->n_part == REAL_PART ? creal(nmod) : cimag(nmod));
			fprintf(f, "%d: num: %.10f model:%.10f\n", i, num_der, mod_der);
		}
	}

	fclose(f);
}
