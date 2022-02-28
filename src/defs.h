
#ifndef DEFS_H
#define DEFS_H

#define REGPRO_VERSION_MAJOR 2
#define REGPRO_VERSION_MINOR 0
#define REGPRO_VERSION_PATCH 2

#define REGISTRATION_VERSION 2

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

#ifndef WIN32
#define HAVE_VASPRINTF 1
#endif

#endif
