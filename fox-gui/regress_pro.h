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

#ifndef REGPRO_REGISTRATION
    bool is_registered() const { return true; }
#endif

    FXGIFIcon appicon;
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
