#ifndef BATCH_WINDOW_H
#define BATCH_WINDOW_H

#include <fx.h>

#include "filelist_table.h"
#include "fit_recipe.h"

class regress_pro_window;

class batch_window : public FXDialogBox {
    FXDECLARE(batch_window)

protected:
    batch_window() {};
private:
    batch_window(const batch_window&);
    batch_window &operator=(const batch_window&);

public:
    batch_window(regress_pro_window *w, FXuint opts=DECOR_ALL,FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,FXint hs=0,FXint vs=0);

    filelist_table *dataset() const { return table; }

    long on_cmd_run_batch(FXObject *, FXSelector, void *);

    enum {
        ID_RUN_BATCH = FXDialogBox::ID_LAST,
        ID_LAST
    };

private:
    int batch_run(fit_recipe *recipe, FXString error_msg);

    regress_pro_window *app_window;
    filelist_table *table;
};

#endif
