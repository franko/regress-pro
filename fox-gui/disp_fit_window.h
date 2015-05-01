#ifndef DISP_FIT_WINDOW_H
#define DISP_FIT_WINDOW_H
#include <fx.h>

#include "fit_panel.h"
#include "fit_window.h"

class disp_fit_manager;

class disp_fit_window : public fit_window {
    FXDECLARE(disp_fit_window)
protected:
    disp_fit_window() {};
private:
    disp_fit_window(const disp_fit_window&);
    disp_fit_window &operator=(const disp_fit_window&);

public:
    disp_fit_window(disp_fit_manager* fit,FXWindow* win,const FXString& name,FXuint opts=DECOR_ALL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,FXint hs=0,FXint vs=0);
    virtual ~disp_fit_window();

    long on_cmd_select(FXObject *, FXSelector, void *);
    long on_cmd_edit_model(FXObject *, FXSelector, void *);
    long on_cmd_save_userlib(FXObject *, FXSelector, void *);

    enum {
        ID_SELECT_REF = fit_window::ID_LAST,
        ID_SELECT_MODEL,
        ID_EDIT_MODEL,
        ID_SAVE_USERLIB,
        ID_LAST
    };

protected:
    FXMenuPane *dispmenu;
    disp_fit_manager *m_fit_manager;
};

#endif
