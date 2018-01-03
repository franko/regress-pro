#ifndef ELLISS_APP_H
#define ELLISS_APP_H

#include <fx.h>
#include "icon.h"

class regress_pro : public FX::FXApp {
    FXDECLARE(regress_pro)
public:
    regress_pro();
    virtual ~regress_pro();

    void setScriptMode(bool mode) { m_script_mode = mode; }
    bool scriptMode() const { return m_script_mode; }
    FXString get_release_string() const;
    FXString get_host_string() const;

    FXGIFIcon appicon;
    FXFont small_font;
    FXFont bold_font;
    FXFont lit_font;
    FXFont monospace_font;
    FXFont big_web_font;
    FXColor blue_web, blue_highlight, red_warning, black;
    FXIcon *delete_icon, *add_icon, *broom_icon;
    FXString spectra_dir, recipe_dir, disp_dir;

private:
    bool m_script_mode;
};

#endif
