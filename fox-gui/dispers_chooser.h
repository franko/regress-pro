#ifndef DISPERS_CHOOSER_H
#define DISPERS_CHOOSER_H

#include <fx.h>

#include "dispers.h"

class dispers_selector {
public:
    virtual ~dispers_selector() {};
    virtual disp_t *get() = 0;
};

class dispers_chooser : public FXDialogBox {
    FXDECLARE(dispers_chooser)
protected:
    dispers_chooser() {};
private:
    dispers_chooser(const dispers_chooser&);
    dispers_chooser &operator=(const dispers_chooser&);

public:
    dispers_chooser(FXApp* a, FXuint opts=DECOR_ALL,FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,FXint hs=0,FXint vs=0);
    virtual ~dispers_chooser();

    // Transfer the ownership of the dispersion.
    disp_t *get_dispersion();

    long on_cmd_category(FXObject *, FXSelector, void *);
    long on_cmd_dispers(FXObject *, FXSelector, void *);

    enum {
        ID_CATEGORY = FXDialogBox::ID_LAST,
        ID_DISPERS,
        ID_LAST
    };

private:
    FXList *catlist;
    FXSwitcher *choose_switcher;
    FXVerticalFrame *vframe;
    FXWindow *dispwin;
    FXWindow *dispwin_anchor;
    dispers_selector *dispers_selectors[4];
    disp_t *current_disp;
};

#endif
