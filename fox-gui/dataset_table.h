#ifndef DATASET_TABLE_H
#define DATASET_TABLE_H

#include <fx.h>

#include "filelist_table.h"
#include "fit_recipe.h"
#include "writer.h"
#include "lexer.h"

/* Used to hold a simple linked list of fit parameters. Each node
 * represent the association of a fit parameter with a given column
 * of the dataset table. */
struct fit_param_node {
    int fp_index;
    int column;
    fit_param_node *next;

    fit_param_node(int index, int col): fp_index(index), column(col), next(NULL) {}
};

struct fit_param_link {
    fit_parameters *params;
    fit_param_node *top;

    fit_param_link(fit_parameters *fps): params(fps), top(NULL) { }

    fit_param_link(): top(NULL) {
        params = fit_parameters_new();
    }

    ~fit_param_link() {
        free_nodes();
        if (params) {
            fit_parameters_free(params);
        }
    }

    // Implement "move" semantics.
    void set_to(fit_param_link *other) {
        params = other->params;
        top = other->top;
        other->top = NULL;
        other->params = NULL;
    }

    void free_nodes() {
        fit_param_node *pnext;
        for (fit_param_node *p = top; p; p = pnext) {
            pnext = p->next;
            delete p;
        }
    }
};

class dataset_table : public filelist_table {
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

    void link_parameter(const fit_param_t *fp, const int column);
    bool get_spectra_list(spectrum *spectra_list[], FXString& error_filename);
    bool get_values(int row, const fit_parameters *fps, double value_array[], int& error_col);
    int samples_number() { return entries_no; }

    long on_cmd_select_column(FXObject *, FXSelector, void *);
    long on_cmd_fit_param(FXObject *, FXSelector, void *);
    long on_cmd_stack_change(FXObject *, FXSelector, void *);
    long on_cmd_stack_shift(FXObject *, FXSelector, void *);

    enum {
        ID_STACK_CHANGE = filelist_table::ID_LAST,
        ID_STACK_SHIFT,
        ID_FIT_PARAM,
        ID_FIT_PARAM_LAST = ID_FIT_PARAM + 256,
        ID_LAST,
    };

    int write(writer_t *w);
    int read_update(lexer_t *l);

private:
    int get_cell_value(int i, int j, double *pvalue);
    void stack_change_update(stack_t *stack);
    void set_column_parameter_name(const fit_param_t *fp, int column);

    FXMenuPane *popupmenu;

    fit_parameters *fit_params;
    int popup_col;

    fit_param_link fplink;
    str_t buffer; // Used to store temporary column name.
};

#endif
