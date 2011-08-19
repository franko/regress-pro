#include <stdio.h>
#include <string.h>

#include "common.h"
#include "elliss-util.h"
#include "error-messages.h"

struct spectrum *
load_ellips_spectrum (const char *filename, struct extra_param *einf)
{
  struct spectrum *spectr = NULL;
  enum spectra_kind spkind;
  FILE *f;
  int iseof, npt, nr;
  str_t ln;

 sr_start:
  f = fopen (filename, "r");
  
  if (f == NULL)
    {
      notify_error_msg (LOADING_FILE_ERROR, "error loading spectra %s",
			filename);
      return NULL;
    }

  str_init (ln, 64);
  str_getline (ln, f);
  if (strstr (CSTR(ln), "SE ALPHA BETA"))
    spkind = AB_ELLISS_SPECTR;
  else if (strstr (CSTR(ln), "SE PSI DELTA"))
    spkind = PSIDEL_ELLISS_SPECTR;
  else
    goto no_good;

  einf->aoi = 71.7;
  einf->analyzer = 25.0;
  einf->numap = 0.0;

  for (;;) {
    iseof = str_getline (ln, f);
    
    if (iseof != 0)
      break;

    nr = sscanf (CSTR(ln), "AOI %lf", & einf->aoi);
    if (nr == 1)
      continue;

    nr = sscanf (CSTR(ln), "NA %lf", & einf->numap);
    if (nr == 1)
      continue;

    nr = sscanf (CSTR(ln), "A %lf", & einf->analyzer);
    if (nr == 1)
      continue;

    break;
  }

  einf->aoi = DEGREE(einf->aoi);
  einf->analyzer = DEGREE(einf->analyzer);
  
  for (npt = 0; ; npt++, iseof = str_getline (ln, f))
    {
      double a[3];
      double *dt[3] = {a, a+1, a+2};
      int k;

      if (spectr)
	{
	  dt[0] = & spectr->lambda[npt];
	  dt[1] = & spectr->data.ab[npt].alpha;
	  dt[2] = & spectr->data.ab[npt].beta;
	}

      while (iseof == 0)
	{
	  k = sscanf (CSTR(ln), "%*s %lf %lf %lf\n", dt[0], dt[1], dt[2]);
	  if (k == 3)
	    break;
	  iseof = str_getline (ln, f);
	}

      if (iseof != 0)
	{
	  if (spectr != NULL)
	    break;

	  if (npt < 2)
	    goto no_good;

	  spectr = emalloc(sizeof(struct spectrum));
	  spectr->kind = spkind;
	  spectr->npt = npt;
	  spectr->lambda  = emalloc (npt * sizeof(double));
	  spectr->data.ab = emalloc (npt * sizeof(struct elliss_ab));

	  fclose (f);
	  str_free (ln);

	  goto sr_start;
	}
    }
  
  spectr->lmin = spectr->lambda[0];
  spectr->lmax = spectr->lambda[spectr->npt - 1];

  str_free (ln);
  fclose (f);

  return spectr;
 no_good:
  notify_error_msg (LOADING_FILE_ERROR, "format of spectra %s is incorrect",
		    filename);
  str_free (ln);
  fclose (f);
  return NULL;
}
