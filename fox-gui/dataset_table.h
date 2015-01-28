#ifndef DATASET_TABLE_H
#define DATASET_TABLE_H

#include <fx.h>

#include "fit_recipe.h"

class dataset_table : public FXTable {
    FXDECLARE(dataset_table)

protected:
    dataset_table() {};
private:
    dataset_table(const dataset_table&);
    dataset_table &operator=(const dataset_table&);

public:
    dataset_table(fit_recipe *rcp, FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_MARGIN,FXint pr=DEFAULT_MARGIN,FXint pt=DEFAULT_MARGIN,FXint pb=DEFAULT_MARGIN);
    virtual ~dataset_table();

    virtual void create();

    void append_filenames(FXString *filenames);

    long on_cmd_select_column(FXObject *, FXSelector, void *);
    long on_cmd_fit_param(FXObject *, FXSelector, void *);

    enum {
        ID_FIT_PARAM = FXTable::ID_LAST,
        ID_FIT_PARAM_LAST = ID_FIT_PARAM + 256,
        ID_LAST,
    };

private:
    FXMenuPane *popupmenu;

    int entries_no;
    fit_recipe *recipe;
    fit_parameters *fit_params;
    int popup_col;
};

#endif
