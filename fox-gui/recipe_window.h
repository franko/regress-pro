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

    void setup_parameters_list(FXComposite *comp);

    long on_cmd_param_select(FXObject*, FXSelector, void*);
    long on_cmd_seed(FXObject*, FXSelector, void*);
    long on_update_seed(FXObject*, FXSelector, void*);
    long on_keypress_parameter(FXObject*, FXSelector, void*);

    enum {
        ID_PARAM_SELECT = FXDialogBox::ID_LAST,
        ID_PARAMETER,
        ID_SEED,
        ID_GRID_MIN,
        ID_GRID_MAX,
        ID_GRID_STEP,
        ID_LAST
    };

private:
    void populate_fit_parameters();
    const fit_param_t *selected_parameter();
    void set_fit_parameter(const fit_param_t *fp, const seed_t *value);
    void update_seed_value(const fit_param_t *fp);
    void set_seed_fields(const seed_t *s);
    void clear_seed_textfield();
    void clear_grid_textfields();
    void fit_list_append_parameter(const fit_param_t *fp, const seed_t *value);
    void fit_list_update_parameter(int i, const fit_param_t *fp, const seed_t *value);

    FXListBox *param_listbox;
    FXTextField *seed_tf;
    FXTextField *grid_min_tf, *grid_max_tf, *grid_step_tf;
    FXList *fit_list;

    fit_recipe *recipe;
    fit_parameters *param_list;
    bool seed_dirty;
};

#endif
