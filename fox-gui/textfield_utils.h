#ifndef TEXTFIELD_UTILS_H
#define TEXTFIELD_UTILS_H

#include <stdlib.h>
#include <fx.h>

template <typename TFloat>
int textfield_get_double(FXTextField *tf, TFloat *pval)
{
	char *tail;
	FXString s = tf->getText();
	double value = strtod(s.text(), &tail);
	if (*tail != 0) return 1;
	*pval = TFloat(value);
	return 0;
}

#endif
