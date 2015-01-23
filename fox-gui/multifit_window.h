#ifndef MULTIFIT_WINDOW_H
#define MULTIFIT_WINDOW_H

#include <fx.h>

#include "fit_recipe.h"

class multifit_window : public FXDialogBox {
    FXDECLARE(multifit_window)

protected:
    multifit_window() {};
private:
    multifit_window(const multifit_window&);
    multifit_window &operator=(const multifit_window&);

public:
    multifit_window(fit_recipe *rcp, FXWindow* w, FXuint opts=DECOR_ALL,FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,FXint hs=0,FXint vs=0);
    virtual ~multifit_window();

    void setup_parameters_list();
    const fit_param_t *selected_parameter() const;

    long on_cmd_add_fit(FXObject *, FXSelector, void *);
    long on_cmd_add_constr(FXObject *, FXSelector, void *);
    long on_select_param(FXObject *, FXSelector, void *);

    enum {
        ID_PARAM_SELECT = FXDialogBox::ID_LAST,
        ID_PARAM_INDIV,
        ID_PARAM_CONSTR,
        ID_ADD_FIT,
        ID_ADD_CONSTR,
        ID_LAST
    };

private:
    fit_recipe *recipe;

    FXListBox *param_listbox;
    FXList *indiv_listbox, *constr_listbox;
    fit_parameters *param_list;
    fit_parameters *iparameters, *cparameters;
};

#endif
