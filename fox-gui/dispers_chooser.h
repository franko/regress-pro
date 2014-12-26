#ifndef DISPERS_CHOOSER_H
#define DISPERS_CHOOSER_H

#endif

#include <fx.h>

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

    long on_cmd_category(FXObject *, FXSelector, void *);

    enum {
        ID_CATEGORY = FXDialogBox::ID_LAST,
        ID_LAST
    };

private:
    FXList *catlist;
    FXSwitcher *choose_switcher;
    FXSwitcher *disp_switcher;
};
