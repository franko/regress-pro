#ifndef ELLISS_APP_H
#define ELLISS_APP_H

#include <fx.h>
#include "icon.h"

class regress_pro : public FX::FXApp {
    FXDECLARE(regress_pro)
public:
    regress_pro();
    virtual ~regress_pro();

    FXGIFIcon appicon;
    FXFont small_font;
    FXFont bold_font;
    FXFont lit_font;
    FXFont monospace_font;
    FXFont big_web_font;
    FXColor blue_web, blue_highlight, red_warning, black;
    FXIcon *delete_icon, *add_icon, *broom_icon;
    FXString spectra_dir, recipe_dir, disp_dir;
};

#endif
