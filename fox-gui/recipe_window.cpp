#include <fxkeys.h>
#include <stdint.h>

#include "recipe_window.h"
#include "fit_params_utils.h"
#include "stack.h"
#include "regress_pro.h"

static void set_numeric_textfield(FXTextField *tf, double value);
static bool range_correct_format(const char *txt, double ps[]);

// Map
FXDEFMAP(recipe_window) recipe_window_map[]= {
    FXMAPFUNC(SEL_COMMAND, recipe_window::ID_PARAM_SELECT, recipe_window::on_cmd_param_select),
    FXMAPFUNC(SEL_KEYPRESS, recipe_window::ID_PARAMETER, recipe_window::on_keypress_parameter),
    FXMAPFUNC(SEL_SELECTED, recipe_window::ID_PARAMETER, recipe_window::on_select_parameter),
    FXMAPFUNCS(SEL_COMMAND, recipe_window::ID_SEED, recipe_window::ID_GRID_STEP, recipe_window::on_cmd_seed),
    FXMAPFUNCS(SEL_UPDATE, recipe_window::ID_SEED, recipe_window::ID_GRID_STEP, recipe_window::on_update_seed),
    FXMAPFUNC(SEL_CHANGED, recipe_window::ID_SPECTRA_RANGE, recipe_window::on_changed_range),
    FXMAPFUNC(SEL_CHANGED, recipe_window::ID_CHISQ_THRESHOLD, recipe_window::on_changed_threshold),
    FXMAPFUNC(SEL_CHANGED, recipe_window::ID_ITERATIONS, recipe_window::on_changed_iterations),
    FXMAPFUNC(SEL_CHANGED, recipe_window::ID_SUBSAMPLE, recipe_window::on_changed_subsampling),
    FXMAPFUNC(SEL_COMMAND, recipe_window::ID_STACK_CHANGE, recipe_window::on_cmd_stack_change),
    FXMAPFUNC(SEL_COMMAND, recipe_window::ID_DELETE, recipe_window::onCmdHide),
    FXMAPFUNC(SEL_COMMAND, recipe_window::ID_MULTI_SAMPLE, recipe_window::on_cmd_multi_sample),
    FXMAPFUNC(SEL_COMMAND, recipe_window::ID_ADD_INDIV, recipe_window::on_cmd_add_indiv),
    FXMAPFUNC(SEL_COMMAND, recipe_window::ID_ADD_CONSTR, recipe_window::on_cmd_add_constr),
    FXMAPFUNC(SEL_SELECTED, recipe_window::ID_PARAM_INDIV, recipe_window::on_select_param),
    FXMAPFUNC(SEL_SELECTED, recipe_window::ID_PARAM_CONSTR, recipe_window::on_select_param),
};

FXIMPLEMENT(recipe_window,FXPacker,recipe_window_map,ARRAYNUMBER(recipe_window_map));

recipe_window::recipe_window(fit_recipe *rcp, FXComposite *p, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXPacker(p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs),
    recipe(rcp), param_list(NULL), seed_dirty(true)
{
    FXVerticalFrame *vf = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXSpring *topspr = new FXSpring(vf, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 70);
    top_frame = new FXHorizontalFrame(topspr, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    FXGroupBox *sgb = new FXGroupBox(top_frame, "Fit Options", GROUPBOX_NORMAL|FRAME_LINE);
    FXMatrix *rmatrix = new FXMatrix(sgb, 2, MATRIX_BY_COLUMNS);
    new FXLabel(rmatrix, "Wavelength Range");
    range_textfield = new FXTextField(rmatrix, 10, this, ID_SPECTRA_RANGE, FRAME_SUNKEN);
    new FXLabel(rmatrix, "ChiSq threshold");
    chisq_textfield = new FXTextField(rmatrix, 5, this, ID_CHISQ_THRESHOLD, FRAME_SUNKEN|TEXTFIELD_REAL);
    new FXLabel(rmatrix, "Max Iterations");
    iter_textfield = new FXTextField(rmatrix, 5, this, ID_ITERATIONS, FRAME_SUNKEN|TEXTFIELD_INTEGER);
    new FXLabel(rmatrix, "Sub sampling");
    subsamp_textfield = new FXTextField(rmatrix, 5, this, ID_SUBSAMPLE, FRAME_SUNKEN|TEXTFIELD_INTEGER);
    multi_sample_button = new FXCheckButton(sgb, "Enable multi-sample", this, ID_MULTI_SAMPLE);

    setup_config_parameters();

    FXGroupBox *lgb = new FXGroupBox(top_frame, "Fit Parameters", GROUPBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_LINE);
    FXVerticalFrame *frame = new FXVerticalFrame(lgb, LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN);
    fit_list = new FXList(frame, this, ID_PARAMETER, LIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    fit_list->setNumVisible(16);

    new FXHorizontalSeparator(vf,SEPARATOR_GROOVE|LAYOUT_FILL_X);

    FXSpring *botspr = new FXSpring(vf, LAYOUT_FILL_X|LAYOUT_FILL_Y, 0, 30);
    FXHorizontalFrame *bhf = new FXHorizontalFrame(botspr, LAYOUT_FILL_Y);

    params_group = new FXGroupBox(bhf, "Fit Parameters", GROUPBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_LINE);
    param_listbox = new FXListBox(params_group, this, ID_PARAM_SELECT);
    param_listbox->setNumVisible(8);

    setup_parameters_list();

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

    list_populate(fit_list, recipe->parameters, recipe->seeds_list, true);

    if (recipe->ms_setup) {
        setup_multi_sample_parameters(false);
    }
}

void recipe_window::setup_config_parameters()
{
    spectral_range *range = &recipe->config->spectr_range;
    FXString text;
    if (range->active) {
        FXString text;
        text.format("%g-%g", range->min, range->max);
        range_textfield->setText(text);
    } else {
        range_textfield->setText("");
    }
    if (recipe->config->threshold_given) {
        text.format("%g", recipe->config->chisq_threshold);
        chisq_textfield->setText(text);
    } else {
        chisq_textfield->setText("");
    }
    text.format("%d", recipe->config->nb_max_iters);
    iter_textfield->setText(text);
    if (recipe->config->subsampling != 1) {
        text.format("%d", recipe->config->subsampling);
        subsamp_textfield->setText(text);
    } else {
        subsamp_textfield->setText("");
    }
}

void recipe_window::setup_parameters_list()
{
    if (param_list) {
        fit_parameters_free(param_list);
    }
    param_list = listbox_populate_all_parameters(param_listbox, recipe->stack);
    seed_dirty = true;
}

void recipe_window::setup_multi_sample_parameters(bool create_elements)
{
    if (!recipe->ms_setup) {
        if (multi_sample_button->getCheck() == TRUE) {
            multi_sample_button->setCheck(FALSE, FALSE);
            disable_multi_sample();
        }
    } else {
        if (multi_sample_button->getCheck() != TRUE) {
            multi_sample_button->setCheck(TRUE, FALSE);
            enable_multi_sample(create_elements);
        } else {
            iparams_listbox->clearItems();
            cparams_listbox->clearItems();
        }
        multi_sample_recipe *ms = recipe->ms_setup;
        for (unsigned i = 0; i < ms->iparameters->number; i++) {
            const fit_param_t *fp = &ms->iparameters->values[i];
            iparams_listbox->appendItem(format_fit_parameter(fp));
        }
        for (unsigned i = 0; i < ms->cparameters->number; i++) {
            const fit_param_t *fp = &ms->cparameters->values[i];
            cparams_listbox->appendItem(format_fit_parameter(fp));
        }
    }
}

recipe_window::~recipe_window()
{
    fit_parameters_free(param_list);
}

const fit_param_t *
recipe_window::selected_parameter()
{
    FXint no = param_listbox->getCurrentItem();
    int index = (intptr_t)(param_listbox->getItemData(no)) - 1;
    return index >= 0 ? &param_list->values[index] : NULL;
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
    int i = fit_parameters_find(recipe->parameters, fp);
    if (i >= 0) {
        set_seed_fields(&recipe->seeds_list->values[i]);
    } else {
        clear_seed_textfield();
        clear_grid_textfields();
    }
}

long
recipe_window::on_cmd_param_select(FXObject* sender, FXSelector, void *)
{
    seed_dirty = true;
    return 1;
}

void
recipe_window::fit_list_append_parameter(const fit_param_t *fp, const seed_t *value)
{
    FXString txt = format_fit_parameter(fp, value);
    fit_list->appendItem(txt);
}

void
recipe_window::fit_list_update_parameter(int i, const fit_param_t *fp, const seed_t *value)
{
    FXString txt = format_fit_parameter(fp, value);
    fit_list->setItem(i, txt);
}

void
recipe_window::set_fit_parameter(const fit_param_t *fp, const seed_t *value)
{
    int i = fit_parameters_find(recipe->parameters, fp);
    if (i >= 0) {
        recipe->seeds_list->values[i] = *value;
        fit_list_update_parameter(i, fp, value);
    } else {
        fit_parameters_add(recipe->parameters, fp);
        seed_list_add(recipe->seeds_list, value);
        fit_list_append_parameter(fp, value);
    }
}

long
recipe_window::on_cmd_seed(FXObject *, FXSelector sel, void *)
{
    const fit_param_t *selfp = selected_parameter();
    if (selfp == NULL) return 0;
    int id = FXSELID(sel);
    seed_t s[1];
    if (id == ID_SEED) {
        s->type = SEED_SIMPLE;
        s->seed = strtod(seed_tf->getText().text(), NULL);
    } else if (id >= ID_GRID_MIN && id <= ID_GRID_STEP) {
        s->type = SEED_RANGE;
        s->min = strtod(grid_min_tf->getText().text(), NULL);
        s->max = strtod(grid_max_tf->getText().text(), NULL);
        s->step = strtod(grid_step_tf->getText().text(), NULL);
    }
    set_fit_parameter(selfp, s);
    return 1;
}

long
recipe_window::on_update_seed(FXObject *, FXSelector, void *)
{
    if (!seed_dirty) return 0;
    const fit_param_t *fp = selected_parameter();
    if (fp) {
        update_seed_value(fp);
        seed_dirty = false;
    }
    return 1;
}

long
recipe_window::on_keypress_parameter(FXObject*, FXSelector sel, void *ptr)
{
    FXEvent* event=(FXEvent*)ptr;
    switch(event->code) {
    case KEY_Delete:
    {
        FXint index = fit_list->getCurrentItem();
        if (index >= 0) {
            fit_list->removeItem(index);
            fit_parameters_remove(recipe->parameters, index);
            seed_list_remove(recipe->seeds_list, index);
            return 1;
        }
    }
    default:
        /* */ ;
    }
    return 0;
}

long
recipe_window::on_changed_range(FXObject *, FXSelector sel, void *ptr)
{
    FXchar *txt = (FXchar *) ptr;
    double ps[2];
    if (range_correct_format(txt, ps)) {
        range_textfield->setTextColor(regressProApp()->black);
        recipe->config->spectr_range.active = 1;
        recipe->config->spectr_range.min = ps[0];
        recipe->config->spectr_range.max = ps[1];
    } else {
        recipe->config->spectr_range.active = 0;
        range_textfield->setTextColor(regressProApp()->red_warning);
    }
    return 1;
}

long
recipe_window::on_changed_threshold(FXObject *, FXSelector sel, void *ptr)
{
    FXchar *txt = (FXchar *) ptr;
    char *tail;
    double v = strtod(txt, &tail);
    if (tail != txt) {
        recipe->config->chisq_threshold = v;
        recipe->config->threshold_given = 1;
    } else {
        recipe->config->threshold_given = 0;
    }
    return 1;
}

long
recipe_window::on_changed_iterations(FXObject *, FXSelector sel, void *ptr)
{
    FXchar *txt = (FXchar *) ptr;
    char *tail;
    long n = strtol(txt, &tail, 10);
    if (tail != txt) {
        recipe->config->nb_max_iters = n;
    } else {
        recipe->config->nb_max_iters = 30;
    }
    return 1;
}

long
recipe_window::on_changed_subsampling(FXObject *, FXSelector sel, void *ptr)
{
    FXchar *txt = (FXchar *) ptr;
    char *tail;
    long n = strtol(txt, &tail, 10);
    if (tail != txt) {
        recipe->config->subsampling = n;
    } else {
        recipe->config->subsampling = 1;
    }
    return 1;
}

long
recipe_window::on_cmd_stack_change(FXObject *, FXSelector, void *)
{
    list_populate(fit_list, recipe->parameters, recipe->seeds_list, true);
    setup_parameters_list();
    return 1;
}

long
recipe_window::on_select_parameter(FXObject *, FXSelector, void *)
{
    FXint index = fit_list->getCurrentItem();
    const fit_param_t *selfp = &recipe->parameters->values[index];
    int fp_index = fit_parameters_find(param_list, selfp);
    listbox_select_parameter(param_listbox, fp_index);
    return 1;
}

void set_numeric_textfield(FXTextField *tf, double value)
{
    FXString text;
    text.format("%g", value);
    tf->setText(text);
}

bool range_correct_format(const char *txt, double ps[])
{
    int nass = sscanf(txt, "%lf-%lf", ps, ps+1);
    return (nass == 2 && ps[1] > ps[0]);
}

void
recipe_window::bind_new_fit_recipe(fit_recipe *rcp)
{
    recipe = rcp;
    list_populate(fit_list, recipe->parameters, recipe->seeds_list, true);
    setup_parameters_list();
    setup_multi_sample_parameters();
    setup_config_parameters();
}

void
recipe_window::enable_multi_sample(bool create_elements)
{
    iparams_group = new FXGroupBox(top_frame, "Sample", GROUPBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_LINE);
    iparams_listbox = new FXList(iparams_group, this, ID_PARAM_INDIV, LIST_SINGLESELECT|LAYOUT_FILL_Y|LAYOUT_FILL_X);
    cparams_group = new FXGroupBox(top_frame, "Constraints", GROUPBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_LINE);
    cparams_listbox = new FXList(cparams_group, this, ID_PARAM_CONSTR, LIST_SINGLESELECT|LAYOUT_FILL_Y|LAYOUT_FILL_X);

    sample_add_button = new FXButton(params_group, "Sample", NULL, this, ID_ADD_INDIV);
    constr_add_button = new FXButton(params_group, "Constraints", NULL, this, ID_ADD_CONSTR);

    if (create_elements) {
        sample_add_button->create();
        constr_add_button->create();
        params_group->recalc();

        iparams_group->create();
        cparams_group->create();
    }
    this->resize(700, 420);
}

void
recipe_window::disable_multi_sample()
{
    delete iparams_group;
    delete cparams_group;
    delete sample_add_button;
    delete constr_add_button;
    params_group->recalc();
    this->resize(540, 420);
}

long
recipe_window::on_cmd_multi_sample(FXObject *, FXSelector, void *ptr)
{
    if (ptr && !recipe->ms_setup) {
        recipe->ms_setup = new multi_sample_recipe();
        enable_multi_sample();
        return 1;
    } else if (!ptr && recipe->ms_setup) {
        delete recipe->ms_setup;
        recipe->ms_setup = NULL;
        disable_multi_sample();
        return 1;
    }
    return 0;
}

long recipe_window::on_cmd_add_indiv(FXObject *, FXSelector, void *)
{
    const fit_param_t *fp = selected_parameter();
    int i = fit_parameters_find(recipe->ms_setup->iparameters, fp);
    if (i < 0) {
        fit_parameters_add(recipe->ms_setup->iparameters, fp);
        iparams_listbox->appendItem(format_fit_parameter(fp));
    }
    return 1;
}

long recipe_window::on_cmd_add_constr(FXObject *, FXSelector, void *)
{
    const fit_param_t *fp = selected_parameter();
    int i = fit_parameters_find(recipe->ms_setup->cparameters, fp);
    if (i < 0) {
        fit_parameters_add(recipe->ms_setup->cparameters, fp);
        cparams_listbox->appendItem(format_fit_parameter(fp));
    }
    return 1;
}


long recipe_window::on_select_param(FXObject *, FXSelector sel, void *)
{
    int id = FXSELID(sel);
    multi_sample_recipe *ms = recipe->ms_setup;
    const fit_param_t *selfp;
    if (id == ID_PARAM_INDIV) {
        FXint index = iparams_listbox->getCurrentItem();
        /* Get the selected fit parameter. */
        selfp = &ms->iparameters->values[index];
    } else {
        FXint index = cparams_listbox->getCurrentItem();
        /* Get the selected fit parameter. */
        selfp = &ms->cparameters->values[index];
    }
    /* Find the index of the fit parameter in the list of all
       possible parameters. */
    int fp_index = fit_parameters_find(param_list, selfp);
    /* Select in the listbox the given parameter. */
    listbox_select_parameter(param_listbox, fp_index);
    return 1;
}
