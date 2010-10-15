
#include <string.h>
#include "batch.h"

int
batch_descr_parse (const char *descr, struct spectra_lst *bspec,
		   int extended)
{
  const char *ptr = descr;
  char ifmt[5];
  char *next;
  int digits, slen;

  if (strchr (descr, (int) '%'))
    return 1;

  str_copy_c (bspec->name, ptr);

  next = strchr (ptr, (int) '#');
  if (! next)
    return 1;
  str_trunc (bspec->name, next - ptr);
  ptr = next;
  while (*next == '#')
    next++;
  digits = next - ptr;
  if (digits <= 0 || digits > 9)
    return 1;
  sprintf (ifmt, "%%0%ii", digits);
  str_append_c (bspec->name, ifmt, 0);
  slen = STR_LENGTH(bspec->name);
  str_append_c (bspec->name, next, 0);

  if (extended)
    {
      ptr = next;
      next = strchr (ptr, (int) '[');
      if (! next)
	return 1;
      slen += next - ptr;
      str_trunc (bspec->name, slen);
      bspec->length = next - descr;

      ptr = next + 1;
      bspec->start = strtol (ptr, &next, 10);
      if (*next != '-')
	return 1;

      ptr = next + 1;
      bspec->stop = strtol (ptr, &next, 10);
      if (*next == ',')
	{
	  ptr = next + 1;
	  bspec->stride = strtol (ptr, &next, 10);
	  if (! next)
	    return 1;
	}
      else
	bspec->stride = 1;

      if (*next == ']')
	return 0;

      return 1;
    }
  else
    {
      bspec->start = bspec->stop = 0;
      bspec->stride = 1;
    }

  return 0;
}

int
get_batch_filename (str_t sname, struct spectra_lst *batch, int *iter)
{
  if (*iter > batch->stop)
    return 0;

  if (batch->single_file)
    str_copy (sname, batch->name);
  else
    str_printf (sname, CSTR(batch->name), *iter);

  *iter += batch->stride;

  return 1;
}
