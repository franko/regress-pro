#ifndef ERROR_MESSAGES_H
#define ERROR_MESSAGES_H

#include "defs.h"
#include "str.h"

__BEGIN_DECLS

enum err_contest {
  SYNTAX_ERROR,
  SCRIPT_ERROR,
  LOADING_FILE_ERROR,
  INVALID_STRATEGY,
  FIT_ERROR
};

extern void     notify_script_err_msg   (enum err_contest c, int fline,
					 int lline, const char * msg, ...);
extern void     notify_error_msg        (enum err_contest c, 
					 const char * msg, ...);
extern void     get_errors_list         (str_t text);
extern void     clean_error_msgs        (void);
extern int      get_script_error_region (int *fline, int *lline);

__END_DECLS

#endif
