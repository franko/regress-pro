#ifndef DISPERS_UI_EDIT_H
#define DISPERS_UI_EDIT_H

#include <fx.h>

#include "dispers.h"
#include "fx_numeric_field.h"

class fx_disp_window : public FXVerticalFrame {
    FXDECLARE(fx_disp_window)

protected:
    fx_disp_window() {};
private:
    fx_disp_window(const fx_disp_window&);
    fx_disp_window &operator=(const fx_disp_window&);

public:
    fx_disp_window(disp_t *d, FXComposite *p,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
    ~fx_disp_window();

    void setup_name();

    virtual void create();
    virtual void setup_dialog() { setup_name(); }
    virtual void add_dispersion_element() {}
    virtual void delete_dispersion_element(int index) {}
    virtual double *map_parameter(int index) { return 0; }

    long on_cmd_value(FXObject*, FXSelector, void *);
    long on_changed_name(FXObject*, FXSelector, void *);
    long on_disp_element_add(FXObject*, FXSelector, void *);
    long on_disp_element_delete(FXObject*, FXSelector, void *);

    enum {
        ID_NAME = FXVerticalFrame::ID_LAST,
        ID_PARAM_0,
        ID_PARAM_LAST = ID_PARAM_0 + 5 * 16,
        ID_DISP_ELEMENT_DELETE,
        ID_DISP_ELEMENT_DELETE_LAST = ID_DISP_ELEMENT_DELETE + 16,
        ID_DISP_ELEMENT_ADD,
        ID_LAST
    };

protected:
    FXIcon *delete_icon, *add_icon;

    // This is just a reference. The class is not owner of this object.
    disp_t *disp;
};

class fx_disp_ho_window : public fx_disp_window {
public:
    fx_disp_ho_window(disp_t *d, FXComposite *p, FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING)
    : fx_disp_window(d, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
    { }

    virtual void setup_dialog();
    virtual void add_dispersion_element();
    virtual void delete_dispersion_element(int index);
    virtual double *map_parameter(int index);
private:
    FXMatrix *matrix;
};

class fx_disp_cauchy_window : public fx_disp_window {
public:
    fx_disp_cauchy_window(disp_t *d, FXComposite *p, FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING)
    : fx_disp_window(d, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
    { }

    virtual void setup_dialog();
    virtual double *map_parameter(int index);
};

// Create a new fx_disp_window (or a derived class instance) that corresponds to the
// given dispersion.
extern fx_disp_window *new_disp_window(disp_t *d, FXComposite *comp);

#endif
