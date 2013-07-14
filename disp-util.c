#include <stdio.h>
#include <math.h>
#include "common.h"
#include "disp-util.h"

int
write_mat_file(const char *filename, const disp_t *d,
               double lmin, double lmax, double lstep)
{
    FILE *f;
    int jinf, jsup, step;
    int j;

    f = fopen(filename, "w");

    if(f == NULL) {
        return 1;
    }

    fprintf(f, "%s\n", CSTR(d->name));
    fprintf(f, "nm\n");
    fprintf(f, "NK\n");

    jinf = rint(lmin * 100.0);
    jsup = rint(lmax * 100.0);
    step = rint(lstep * 100.0);

    for(j = jinf; j <= jsup; j += step) {
        double lambda = j / 100.0;
        cmpl n = n_value(d, lambda);

        fprintf(f, "%7.2f\t%.7f\t%.7f\n", lambda, creal(n), - cimag(n));
    }

    fclose(f);

    return 0;
}
