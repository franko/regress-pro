#ifndef DISPERS_CHOOSER_H
#define DISPERS_CHOOSER_H

#include <fx.h>

#include "dispers.h"

class fx_dispers_selector : public FXHorizontalFrame {
    FXDECLARE(fx_dispers_selector)
protected:
    fx_dispers_selector() {};
private:
    fx_dispers_selector(const fx_dispers_selector&);
    fx_dispers_selector &operator=(const fx_dispers_selector&);
public:
    fx_dispers_selector(FXWindow *chooser, FXComposite *p,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING)
    : FXHorizontalFrame(p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
    { }

    virtual disp_t *get_dispersion() { return NULL; }
    virtual void reset() { }
};

class dispers_chooser : public FXDialogBox {
    FXDECLARE(dispers_chooser)
protected:
    dispers_chooser() {};
private:
    dispers_chooser(const dispers_chooser&);
    dispers_chooser &operator=(const dispers_chooser&);

public:
    dispers_chooser(FXWindow *win, FXuint opts=DECOR_ALL,FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,FXint hs=0,FXint vs=0);
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
    void clear_dispwin();
    void replace_dispwin(FXWindow *new_dispwin);
    FXWindow *new_dispwin_dummy(FXComposite *frame);

    void release_current_disp()
    {
        if (current_disp) {
            disp_free(current_disp);
        }
        current_disp = NULL;
    }

    fx_dispers_selector *selector_frame(int cat)
    {
        return (fx_dispers_selector *) choose_switcher->childAtIndex(cat);
    }

    FXList *catlist;
    FXSwitcher *choose_switcher;
    FXVerticalFrame *vframe;
    FXWindow *dispwin;
    FXWindow *dispwin_anchor, *dispwin_dummy;
    disp_t *current_disp;
};

#endif
