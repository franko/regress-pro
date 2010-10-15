
/*
  $Id: batch.h,v 1.1 2006/07/12 22:57:45 francesco Exp $
 */

#ifndef BATCH_H
#define BATCH_H

#include "defs.h"
#include "str.h"

__BEGIN_DECLS

struct spectra_lst {
  int single_file;
  str_t name;
  int start, stop;
  int stride;
  int length;
};

extern int get_batch_filename (str_t sname, struct spectra_lst *batch,
			       int *iter);

extern int batch_descr_parse (const char *descr, struct spectra_lst *bspec,
			      int extended);

__END_DECLS

#endif

