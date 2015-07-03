#ifndef DATASET_WINDOW_H
#define DATASET_WINDOW_H

#include <fx.h>

#include "dataset_table.h"
#include "fit_recipe.h"

class dataset_window : public FXDialogBox {
    FXDECLARE(dataset_window)

protected:
    dataset_window() {};
private:
    dataset_window(const dataset_window&);
    dataset_window &operator=(const dataset_window&);

public:
    dataset_window(fit_recipe *rcp, FXWindow *w, FXuint opts=DECOR_ALL,FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,FXint hs=0,FXint vs=0);

    dataset_table *dataset() const { return table; }

    enum {
        ID_TABLE = FXDialogBox::ID_LAST,
        ID_LAST,
    };

private:
    dataset_table *table;
};

#endif
