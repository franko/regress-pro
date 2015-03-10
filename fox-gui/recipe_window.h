#ifndef RECIPE_WINDOW_H
#define RECIPE_WINDOW_H

#include <fx.h>
#include "fit_recipe.h"

struct multi_sample_recipe {
    multi_sample_recipe();
    ~multi_sample_recipe();

    fit_parameters *iparameters;
    fit_parameters *cparameters;
};

class recipe_window : public FXDialogBox {
    FXDECLARE(recipe_window)

protected:
    recipe_window() {};
private:
    recipe_window(const recipe_window&);
    recipe_window &operator=(const recipe_window&);

public:
    recipe_window(fit_recipe *r, FXWindow* w, FXuint opts=DECOR_ALL,FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,FXint hs=0,FXint vs=0);
    virtual ~recipe_window();

    void setup_parameters_list();
    void setup_config_parameters();
    void bind_new_fit_recipe(fit_recipe *rcp);

    long on_cmd_param_select(FXObject*, FXSelector, void*);
    long on_cmd_seed(FXObject*, FXSelector, void*);
    long on_update_seed(FXObject*, FXSelector, void*);
    long on_keypress_parameter(FXObject*, FXSelector, void*);
    long on_changed_range(FXObject*, FXSelector, void*);
    long on_changed_threshold(FXObject*, FXSelector, void*);
    long on_changed_iterations(FXObject*, FXSelector, void*);
    long on_changed_subsampling(FXObject*, FXSelector, void*);
    long on_cmd_stack_change(FXObject*, FXSelector, void*);
    long on_select_parameter(FXObject*, FXSelector, void*);
    long on_cmd_multi_sample(FXObject *, FXSelector, void *ptr);
    long on_cmd_add_indiv(FXObject *, FXSelector, void *);
    long on_cmd_add_constr(FXObject *, FXSelector, void *);
    long on_select_param(FXObject *, FXSelector sel, void *);

    enum {
        ID_PARAM_SELECT = FXDialogBox::ID_LAST,
        ID_PARAMETER,
        ID_SEED,
        ID_GRID_MIN,
        ID_GRID_MAX,
        ID_GRID_STEP,
        ID_SPECTRA_RANGE,
        ID_CHISQ_THRESHOLD,
        ID_ITERATIONS,
        ID_SUBSAMPLE,
        ID_STACK_CHANGE,
        ID_MULTI_SAMPLE,
        ID_PARAM_INDIV,
        ID_PARAM_CONSTR,
        ID_ADD_INDIV,
        ID_ADD_CONSTR,
        ID_LAST
    };

private:
    const fit_param_t *selected_parameter();
    void set_fit_parameter(const fit_param_t *fp, const seed_t *value);
    void update_seed_value(const fit_param_t *fp);
    void set_seed_fields(const seed_t *s);
    void clear_seed_textfield();
    void clear_grid_textfields();
    void fit_list_append_parameter(const fit_param_t *fp, const seed_t *value);
    void fit_list_update_parameter(int i, const fit_param_t *fp, const seed_t *value);
    void enable_multi_sample();

    FXHorizontalFrame *top_frame;
    FXGroupBox *params_group;
    FXListBox *param_listbox;
    FXTextField *seed_tf;
    FXTextField *grid_min_tf, *grid_max_tf, *grid_step_tf;
    FXList *fit_list;
    FXTextField *range_textfield, *chisq_textfield, *iter_textfield, *subsamp_textfield;

    fit_recipe *recipe;
    fit_parameters *param_list;
    multi_sample_recipe *ms_recipe;
    bool seed_dirty;

    FXList *iparams_listbox, *cparams_listbox;
};

#endif
