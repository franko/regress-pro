#ifndef ELLISS_APP_H
#define ELLISS_APP_H

#include <fx.h>
#include "icon.h"

#include "registered_app.h"

template <class base_app>
class elliss_app_gen : public base_app {
public:
    FXGIFIcon appicon;

    elliss_app_gen() :
        base_app("Regress Pro", "Francesco Abbate"),
        appicon(this, regressproicon)
    { }
};

#ifdef REGPRO_REGISTRATION

typedef elliss_app_gen<registered_app> elliss_app;

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

typedef elliss_app_gen<FX::FXApp> base_type;

class elliss_app : public base_type {
public:
    elliss_app() : base_type() {};

    bool is_registered() const {
        return true;
    }
};

inline void reg_check_point(FXId *win) { }
inline void reg_form(FXId *win) { }

#endif

#endif
