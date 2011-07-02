#ifndef REGISTRATION_MANAGER_H
#define REGISTRATION_MANAGER_H

#include <stdbool.h>

#include "defs.h"

__BEGIN_DECLS

bool registration_check(const char *user_name, const char *user_email,
			int product_version, const char *key);

__END_DECLS

#endif
