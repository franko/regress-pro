#ifndef DISPERS_UI_EDIT_H
#define DISPERS_UI_EDIT_H

#include <fx.h>

#include "dispers.h"
#include "fx_numeric_field.h"

class fx_disp_ho_window : public FXHorizontalFrame {
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

    long on_cmd_value(FXObject*, FXSelector sel, void *);

    enum {
        ID_PARAM_0 = FXHorizontalFrame::ID_LAST,
        ID_LAST
    };

private:
    fx_numeric_field **field;
    disp_t *m_disp;
};

#endif
