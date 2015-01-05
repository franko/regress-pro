#include "recipe_window.h"
#include "fit-params.h"
#include "stack.h"

static void set_numeric_textfield(FXTextField *tf, double value);

// Map
FXDEFMAP(recipe_window) recipe_window_map[]= {
    FXMAPFUNC(SEL_COMMAND, recipe_window::ID_PARAMETER, recipe_window::on_cmd_parameter),
    FXMAPFUNCS(SEL_COMMAND, recipe_window::ID_SEED, recipe_window::ID_GRID_STEP, recipe_window::on_cmd_seed),
};

FXIMPLEMENT(recipe_window,FXDialogBox,recipe_window_map,ARRAYNUMBER(recipe_window_map));

recipe_window::recipe_window(fit_recipe *rcp, FXApp* a, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(a, "Recipe Edit", opts, 0, 0, 540, 420, pl, pr, pt, pb, hs, vs),
    recipe(rcp)
{
    FXVerticalFrame *vf = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXSpring *topspr = new FXSpring(vf, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 70);
    FXHorizontalFrame *tophf = new FXHorizontalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    new FXHorizontalSeparator(vf,SEPARATOR_GROOVE|LAYOUT_FILL_X);

    FXSpring *botspr = new FXSpring(vf, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 30);

    FXHorizontalFrame *bhf = new FXHorizontalFrame(botspr, LAYOUT_FILL_Y);

    setup_parameters_list(bhf);

    FXGroupBox *group = new FXGroupBox(bhf, "Initial Fit Value", GROUPBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_LINE);
    FXVerticalFrame *seedvf = new FXVerticalFrame(group, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    seed_tf = new FXTextField(seedvf, 12, this, ID_SEED, FRAME_SUNKEN|TEXTFIELD_REAL|TEXTFIELD_ENTER_ONLY);

    FXMatrix *matrix = new FXMatrix(seedvf, 3, MATRIX_BY_COLUMNS|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    new FXLabel(matrix, "Grid Min", NULL, LAYOUT_FILL_ROW);
    new FXLabel(matrix, "Grid Max", NULL, LAYOUT_FILL_ROW);
    new FXLabel(matrix, "Grid Step", NULL, LAYOUT_FILL_ROW);

    grid_min_tf = new FXTextField(matrix, 10, this, ID_GRID_MIN, LAYOUT_FILL_ROW|FRAME_SUNKEN|TEXTFIELD_REAL|TEXTFIELD_ENTER_ONLY);
    grid_max_tf = new FXTextField(matrix, 10, this, ID_GRID_MAX, LAYOUT_FILL_ROW|FRAME_SUNKEN|TEXTFIELD_REAL|TEXTFIELD_ENTER_ONLY);
    grid_step_tf = new FXTextField(matrix, 10, this, ID_GRID_STEP, LAYOUT_FILL_ROW|FRAME_SUNKEN|TEXTFIELD_REAL|TEXTFIELD_ENTER_ONLY);

    fit_params = fit_parameters_new();
    fit_seeds = seed_list_new();
}

void recipe_window::setup_parameters_list(FXComposite *comp)
{
    param_list = fit_parameters_new();
    stack_get_all_parameters(recipe->stack, param_list);

    fit_seeds = seed_list_new();
    fit_params = fit_parameters_new();

    FXGroupBox *group = new FXGroupBox(comp, "Fit Parameters", GROUPBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_LINE);
    FXListBox *list = new FXListBox(group, this, ID_PARAMETER);
    list->setNumVisible(8);
    str_t pname;
    str_init(pname, 16);
    int current_layer = 0;
    for (size_t i = 0; i < param_list->number; i++) {
        fit_param_t *fp = &param_list->values[i];
        if (fp->id == PID_LAYER_N && fp->layer_nb != current_layer) {
            str_printf(pname, "-- layer %d", fp->layer_nb);
            list->appendItem(CSTR(pname));
            current_layer = fp->layer_nb;
        }
        get_param_name(fp, pname);
        list->appendItem(CSTR(pname), NULL, (void*) (i + 1));
    }
    str_free(pname);
    param_listbox = list;
}

recipe_window::~recipe_window()
{
    fit_parameters_free(param_list);
    fit_parameters_free(fit_params);
    seed_list_free(fit_seeds);
}

void
recipe_window::clear_grid_textfields()
{
    grid_min_tf->setText("");
    grid_max_tf->setText("");
    grid_step_tf->setText("");
}

void
recipe_window::clear_seed_textfield()
{
    seed_tf->setText("");
}

void
recipe_window::set_seed_fields(const seed_t *s)
{
    if (s->type == SEED_SIMPLE) {
        set_numeric_textfield(seed_tf, s->seed);
        clear_grid_textfields();
    } else if (s->type == SEED_RANGE) {
        clear_seed_textfield();
        set_numeric_textfield(grid_min_tf, s->min);
        set_numeric_textfield(grid_max_tf, s->max);
        set_numeric_textfield(grid_step_tf, s->step);
    } else {
        clear_seed_textfield();
        clear_grid_textfields();
    }
}

void
recipe_window::update_seed_value(const fit_param_t *fp)
{
    int i = fit_parameters_find(fit_params, fp);
    if (i >= 0) {
        set_seed_fields(&fit_seeds->values[i]);
    } else {
        clear_seed_textfield();
        clear_grid_textfields();
    }
}

long
recipe_window::on_cmd_parameter(FXObject* sender, FXSelector, void *ptr)
{
    FXint no = (FXint)(FXival)ptr;
    parameter_index = (FXint)(param_listbox->getItemData(no)) - 1;
    const fit_param_t *fp = &param_list->values[parameter_index];
    if (parameter_index >= 0) {
        update_seed_value(fp);
    }
    return 1;
}

void
recipe_window::set_fit_parameter(const fit_param_t *fp, const seed_t *value)
{
    int i = fit_parameters_find(fit_params, fp);
    if (i >= 0) {
        fit_seeds->values[i] = *value;
    } else {
        fit_parameters_add(fit_params, fp);
        seed_list_add(fit_seeds, value);
    }
}

long
recipe_window::on_cmd_seed(FXObject* sender, FXSelector sel, void *)
{
    if (parameter_index < 0) return 0;
    int id = FXSELID(sel);
    seed_t s;
    if (id == ID_SEED) {
        s.type = SEED_SIMPLE;
        s.seed = strtod(seed_tf->getText().text(), NULL);
    } else if (id >= ID_GRID_MIN && id <= ID_GRID_STEP) {
        s.type = SEED_RANGE;
        s.min = strtod(grid_min_tf->getText().text(), NULL);
        s.max = strtod(grid_max_tf->getText().text(), NULL);
        s.step = strtod(grid_step_tf->getText().text(), NULL);
    }
    set_fit_parameter(&param_list->values[parameter_index], &s);
    return 1;
}

void set_numeric_textfield(FXTextField *tf, double value)
{
    FXString text;
    text.format("%g", value);
    tf->setText(text);
}
