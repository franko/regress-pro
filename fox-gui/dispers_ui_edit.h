#ifndef DISPERS_UI_EDIT_H
#define DISPERS_UI_EDIT_H

#include <fx.h>

#include "dispers.h"
#include "fx_numeric_field.h"

class fx_disp_ho_window : public FXVerticalFrame {
    FXDECLARE(fx_disp_ho_window)

protected:
    fx_disp_ho_window() {};
private:
    fx_disp_ho_window(const fx_disp_ho_window&);
    fx_disp_ho_window &operator=(const fx_disp_ho_window&);

public:
    fx_disp_ho_window(disp_t *d, FXComposite* p, FXuint opts=0);
    ~fx_disp_ho_window();

    disp_t *disp() const { return m_disp; }

    long on_cmd_value(FXObject*, FXSelector, void *);
    long on_changed_name(FXObject*, FXSelector, void *);

    enum {
        ID_NAME = FXVerticalFrame::ID_LAST,
        ID_PARAM_0,
        ID_LAST
    };

private:
    fx_numeric_field **field;
    disp_t *m_disp;
};
class fx_disp_cauchy_window : public FXVerticalFrame {
    FXDECLARE(fx_disp_cauchy_window)

protected:
    fx_disp_cauchy_window() {};
private:
    fx_disp_cauchy_window(const fx_disp_cauchy_window&);
    fx_disp_cauchy_window &operator=(const fx_disp_cauchy_window&);

public:
    fx_disp_cauchy_window(disp_t *d, FXComposite* p, FXuint opts=0);
    ~fx_disp_cauchy_window();

    disp_t *disp() const { return m_disp; }

    long on_cmd_value(FXObject*, FXSelector, void *);
    long on_changed_name(FXObject*, FXSelector, void *);

    enum {
        ID_NAME = FXVerticalFrame::ID_LAST,
        ID_PARAM_0,
        ID_LAST
    };

private:
    fx_numeric_field *n0, *n1, *n2;
    fx_numeric_field *k0, *k1, *k2;
    disp_t *m_disp;
};

#endif
