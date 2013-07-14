#ifndef DISP_CHOOSER_H
#define DISP_CHOOSER_H

#include <fx.h>

#include "defs.h"
#include "symtab.h"
#include "disp-fit-engine.h"

bool disp_chooser(FXApp *app, struct symtab *symtab, struct disp_fit_engine *fit);

#endif
