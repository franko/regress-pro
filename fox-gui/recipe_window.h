#ifndef RECIPE_WINDOW_H
#define RECIPE_WINDOW_H

#include <fx.h>
#include "fit_recipe.h"

class regress_pro;

class recipe_window : public FXPacker {
    FXDECLARE(recipe_window)

protected:
    recipe_window() {};
private:
    recipe_window(const recipe_window&);
    recipe_window &operator=(const recipe_window&);

public:
    recipe_window(fit_recipe *r, FXComposite *p, FXuint opts=0, FXint x=0, FXint y=0, FXint w=0, FXint h=0, FXint pl=DEFAULT_SPACING, FXint pr=DEFAULT_SPACING, FXint pt=DEFAULT_SPACING, FXint pb=DEFAULT_SPACING, FXint hs=DEFAULT_SPACING, FXint vs=DEFAULT_SPACING);
    virtual ~recipe_window();

    regress_pro* regressProApp() const { return (regress_pro*) getApp(); }

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
    long on_keypress_param_select(FXObject *, FXSelector sel, void *);

    enum {
        ID_PARAM_SELECT = FXPacker::ID_LAST,
        ID_PARAMETER,
        ID_SEED,
        ID_RANGE,
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
    seed_t get_seed_from_ui(const fit_param_t *fp);
    void set_fit_parameter(const fit_param_t *fp, const seed_t *value);
    void update_seed_value(const fit_param_t *fp);
    void set_seed_fields(const seed_t *s);
    void clear_seed_textfield();
    void clear_range_textfield();
    void fit_list_append_parameter(const fit_param_t *fp, const seed_t *value);
    void fit_list_update_parameter(int i, const fit_param_t *fp, const seed_t *value);
    void setup_parameters_list();
    void setup_config_parameters();
    void enable_multi_sample(bool create_elements = true);
    void setup_multi_sample_parameters(bool create_elements = true);
    void disable_multi_sample();

    FXHorizontalFrame *top_frame;
    FXPacker *params_group;
    FXListBox *param_listbox;
    FXTextField *seed_tf;
    FXTextField *range_tf;
    FXList *fit_list;
    FXTextField *range_textfield, *chisq_textfield, *iter_textfield, *subsamp_textfield;

    fit_recipe *recipe;
    fit_parameters *param_list;
    bool seed_dirty;

    FXGroupBox *iparams_group, *cparams_group;
    FXButton *sample_add_button, *constr_add_button;
    FXList *iparams_listbox, *cparams_listbox;
    FXCheckButton *multi_sample_button;
};

#endif
