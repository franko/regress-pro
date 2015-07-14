#ifndef ELLISS_APP_H
#define ELLISS_APP_H

#include <fx.h>
#include "icon.h"


#ifdef REGPRO_REGISTRATION
#include "registered_app.h"
#else
typedef FX::FXApp registered_app;
#endif

class regress_pro : public registered_app {
    FXDECLARE(regress_pro)
public:
    regress_pro();
    virtual ~regress_pro();

#ifndef REGPRO_REGISTRATION
    bool is_registered() const { return true; }
#endif

    FXGIFIcon appicon;
    FXFont small_font;
    FXFont bold_font;
    FXFont monospace_font;
    FXFont big_web_font;
    FXColor blue_web, blue_highlight, red_warning, black;
    FXIcon *delete_icon, *add_icon;
};

#ifdef REGPRO_REGISTRATION

inline void reg_check_point(FXId *win)
{
    FXuint sel = FXSEL(SEL_COMMAND, registered_app::ID_REGISTER_MARK);
    FXApp *app = win->getApp();
    app->handle(win, sel, NULL);
}

inline void reg_form(FXId *win)
{
    FXuint sel = FXSEL(SEL_COMMAND, registered_app::ID_REGISTER_ASK);
    FXApp *app = win->getApp();
    app->handle(win, sel, NULL);
}

#else

inline void reg_check_point(FXId *win) { }
inline void reg_form(FXId *win) { }

#endif

#endif
