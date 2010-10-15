
/*
  $Id: defs.h,v 1.3 2006/10/30 22:55:13 francesco Exp $
 */

#ifndef DEFS_H
#define DEFS_H

#define VERSION_MAJOR 1
#define VERSION_MINOR 3
#define VERSION_PATCH 2

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

#endif
