#ifndef RECIPE_WINDOW_H
#define RECIPE_WINDOW_H

#include <fx.h>
#include "fit_recipe.h"

class recipe_window : public FXDialogBox {
    FXDECLARE(recipe_window)

protected:
    recipe_window() {};
private:
    recipe_window(const recipe_window&);
    recipe_window &operator=(const recipe_window&);

public:
    recipe_window(fit_recipe *r, FXApp* a, FXuint opts=DECOR_ALL,FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,FXint hs=0,FXint vs=0);
    virtual ~recipe_window();

private:
    fit_recipe *recipe;
};

#endif
