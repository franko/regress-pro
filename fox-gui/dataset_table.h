#ifndef DATASET_TABLE_H
#define DATASET_TABLE_H

#include <fx.h>

#include "fit_recipe.h"

/* Used to hold a simple linked list of fit parameters. Each node
 * represent the association of a fit parameter with a given column
 * of the dataset table. */
struct fit_param_node;

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
    void link_parameter(const fit_param_t *fp, int column);
    bool get_spectra_list(spectrum *spectra_list[], FXString& error_filename);
    bool get_values(int row, const fit_parameters *fps, double value_array[], int& error_col);
    int samples_number() { return entries_no; }

    long on_cmd_select_column(FXObject *, FXSelector, void *);
    long on_cmd_fit_param(FXObject *, FXSelector, void *);
    long on_cmd_stack_change(FXObject *, FXSelector, void *);

    enum {
        ID_STACK_CHANGE = FXTable::ID_LAST,
        ID_FIT_PARAM,
        ID_FIT_PARAM_LAST = ID_FIT_PARAM + 256,
        ID_LAST,
    };

private:
    void stack_change_update(stack_t *stack);

    FXMenuPane *popupmenu;

    int entries_no;
    fit_parameters *fit_params;
    int popup_col;

    fit_param_node *fplink;
};

#endif
