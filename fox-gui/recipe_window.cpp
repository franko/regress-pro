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
    FXMAPFUNC(SEL_KEYPRESS, recipe_window::ID_PARAM_SELECT, recipe_window::on_keypress_param_select),
    FXMAPFUNC(SEL_KEYPRESS, recipe_window::ID_PARAMETER, recipe_window::on_keypress_parameter),
    FXMAPFUNC(SEL_KEYPRESS, recipe_window::ID_PARAM_INDIV, recipe_window::on_keypress_parameter),
    FXMAPFUNC(SEL_KEYPRESS, recipe_window::ID_PARAM_CONSTR, recipe_window::on_keypress_parameter),
    FXMAPFUNC(SEL_SELECTED, recipe_window::ID_PARAMETER, recipe_window::on_select_parameter),
    FXMAPFUNCS(SEL_COMMAND, recipe_window::ID_SEED, recipe_window::ID_RANGE, recipe_window::on_cmd_seed),
    FXMAPFUNCS(SEL_UPDATE, recipe_window::ID_SEED, recipe_window::ID_RANGE, recipe_window::on_update_seed),
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
    recipe(rcp), param_list(nullptr), seed_dirty(true)
{
    top_frame = new FXHorizontalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    FXVerticalFrame *pframe = new FXVerticalFrame(top_frame);
    FXGroupBox *sgb = new FXGroupBox(pframe, "Fit Options", GROUPBOX_NORMAL|FRAME_GROOVE);
    FXMatrix *rmatrix = new FXMatrix(sgb, 2, MATRIX_BY_COLUMNS);
    new FXLabel(rmatrix, "Wavelength Range");
    range_textfield = new FXTextField(rmatrix, 10, this, ID_SPECTRA_RANGE, FRAME_SUNKEN);
    range_textfield->setTipText("Spectra Range in nanometers (\"min-max\")");
    new FXLabel(rmatrix, "ChiSq threshold");
    chisq_textfield = new FXTextField(rmatrix, 5, this, ID_CHISQ_THRESHOLD, FRAME_SUNKEN|TEXTFIELD_REAL);
    chisq_textfield->setTipText("Chi Square threshold used in grid search");
    new FXLabel(rmatrix, "Max Iterations");
    iter_textfield = new FXTextField(rmatrix, 5, this, ID_ITERATIONS, FRAME_SUNKEN|TEXTFIELD_INTEGER);
    iter_textfield->setTipText("Maximum number of iterations for fitting procedure");
    new FXLabel(rmatrix, "Sub sampling");
    subsamp_textfield = new FXTextField(rmatrix, 5, this, ID_SUBSAMPLE, FRAME_SUNKEN|TEXTFIELD_INTEGER);
    subsamp_textfield->setTipText("Enable subsampling of spectra to speedup calculations");
    multi_sample_button = new FXCheckButton(sgb, "Enable multi-sample", this, ID_MULTI_SAMPLE);

    setup_config_parameters();

    FXSpring *fpspr = new FXSpring(top_frame, LAYOUT_FILL_X|LAYOUT_FILL_Y, 42, 0);
    FXGroupBox *lgb = new FXGroupBox(fpspr, "Fit Parameters", GROUPBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_GROOVE);
    fit_list = new FXList(lgb, this, ID_PARAMETER, LIST_SINGLESELECT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    fit_list->setNumVisible(4);

    FXGroupBox *fpgroup = new FXGroupBox(pframe, "Add Fit Parameters", GROUPBOX_NORMAL|LAYOUT_FILL_Y|FRAME_GROOVE);
    params_group = new FXPacker(fpgroup, LAYOUT_FILL_Y|LAYOUT_SIDE_LEFT);
    param_listbox = new FXListBox(params_group, this, ID_PARAM_SELECT, FRAME_SUNKEN|LISTBOX_NORMAL|LAYOUT_FIX_WIDTH, 0, 0, 120, 0);
    param_listbox->setNumVisible(8);

    acquisition_set_default(&known_acquisition);
    setup_parameters_list();

    FXMatrix *matrix = new FXMatrix(fpgroup, 2, MATRIX_BY_COLUMNS|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_SIDE_RIGHT);
    new FXLabel(matrix, "Value", nullptr, LAYOUT_FILL_ROW);
    seed_tf = new FXTextField(matrix, 8, this, ID_SEED, FRAME_SUNKEN|TEXTFIELD_REAL|TEXTFIELD_ENTER_ONLY);
    seed_tf->setTipText("Initial value of fitting parameter");
    new FXLabel(matrix, "Range", nullptr, LAYOUT_FILL_ROW);
    range_tf = new FXTextField(matrix, 8, this, ID_RANGE, LAYOUT_FILL_ROW|FRAME_SUNKEN|TEXTFIELD_REAL|TEXTFIELD_ENTER_ONLY);
    range_tf->setTipText("Optional range used in grid search");

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
    param_list = listbox_populate_all_parameters(param_listbox, recipe->stack, &known_acquisition);
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
        for (int i = 0; i < ms->iparameters->number; i++) {
            const fit_param_t *fp = &ms->iparameters->at(i);
            iparams_listbox->appendItem(format_fit_parameter(fp));
        }
        for (int i = 0; i < ms->cparameters->number; i++) {
            const fit_param_t *fp = &ms->cparameters->at(i);
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
    if (no >= 0) {
        int index = (intptr_t)(param_listbox->getItemData(no)) - 1;
        return index >= 0 ? &param_list->at(index) : nullptr;
    }
    return nullptr;
}

void
recipe_window::clear_range_textfield()
{
    range_tf->setText("");
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
        clear_range_textfield();
    } else if (s->type == SEED_RANGE) {
        set_numeric_textfield(seed_tf, s->seed);
        set_numeric_textfield(range_tf, s->delta);
    } else {
        clear_seed_textfield();
        clear_range_textfield();
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
        clear_range_textfield();
    }
}

long
recipe_window::on_cmd_param_select(FXObject* sender, FXSelector, void *)
{
    seed_dirty = true;
    // seed_tf->setFocus();
    return 1;
}

long
recipe_window::on_keypress_param_select(FXObject*, FXSelector sel, void *ptr)
{
    FXEvent* event=(FXEvent*)ptr;
    switch(event->code) {
    case KEY_Return:
        return handle(this, FXSEL(SEL_COMMAND, ID_SEED), nullptr);
    default:
        /* */ ;
    }
    return 0;
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

seed_t recipe_window::get_seed_from_ui(const fit_param_t *fp)
{
    seed_t s;
    if (range_tf->getText() == "") {
        if (seed_tf->getText() == "") {
            s.type = SEED_UNDEF;
        } else {
            s.type = SEED_SIMPLE;
            s.seed = strtod(seed_tf->getText().text(), nullptr);
        }
    } else {
        s.type = SEED_RANGE;
        s.seed = strtod(seed_tf->getText().text(), nullptr);
        s.delta = strtod(range_tf->getText().text(), nullptr);
    }
    return s;
}

long
recipe_window::on_cmd_seed(FXObject *, FXSelector, void *)
{
    const fit_param_t *selfp = selected_parameter();
    if (selfp == nullptr) return 0;
    seed_t s = get_seed_from_ui(selfp);
    set_fit_parameter(selfp, &s);
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
recipe_window::on_keypress_parameter(FXObject *sender, FXSelector sel, void *ptr)
{
    FXEvent* event=(FXEvent*)ptr;
    switch(event->code) {
    case KEY_Delete:
    {
        if (sender == fit_list) {
            FXint index = fit_list->getCurrentItem();
            if (index < 0) return 0;
            fit_list->removeItem(index);
            fit_parameters_remove(recipe->parameters, index);
            seed_list_remove(recipe->seeds_list, index);
            return 1;
        } else if (sender == iparams_listbox || sender == cparams_listbox) {
            FXList *list = (FXList *) sender;
            fit_parameters *fps = (sender == iparams_listbox ? recipe->ms_setup->iparameters : recipe->ms_setup->cparameters);
            FXint index = list->getCurrentItem();
            if (index < 0) return 0;
            list->removeItem(index);
            fit_parameters_remove(fps, index);
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
    const fit_param_t *selfp = &recipe->parameters->at(index);
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
    iparams_spring = new FXSpring(top_frame, LAYOUT_FILL_X|LAYOUT_FILL_Y, 29, 0);
    FXGroupBox *iparams_group = new FXGroupBox(iparams_spring, "Sample", GROUPBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_GROOVE);
    iparams_listbox = new FXList(iparams_group, this, ID_PARAM_INDIV, LIST_SINGLESELECT|LAYOUT_FILL_Y|LAYOUT_FILL_X);
    cparams_spring = new FXSpring(top_frame, LAYOUT_FILL_X|LAYOUT_FILL_Y, 29, 0);
    FXGroupBox *cparams_group = new FXGroupBox(cparams_spring, "Constraints", GROUPBOX_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_GROOVE);
    cparams_listbox = new FXList(cparams_group, this, ID_PARAM_CONSTR, LIST_SINGLESELECT|LAYOUT_FILL_Y|LAYOUT_FILL_X);


    ms_params_frame = new FXHorizontalFrame(params_group, LAYOUT_FILL_Y);
    new FXButton(ms_params_frame, "sample", nullptr, this, ID_ADD_INDIV, BUTTON_NORMAL|LAYOUT_FILL_X);
    new FXButton(ms_params_frame, "constr", nullptr, this, ID_ADD_CONSTR, BUTTON_NORMAL|LAYOUT_FILL_X);

    if (create_elements) {
        ms_params_frame->create();
        params_group->recalc();

        iparams_spring->create();
        cparams_spring->create();
        top_frame->recalc();
    }
}

void
recipe_window::disable_multi_sample()
{
    delete iparams_spring;
    delete cparams_spring;
    delete ms_params_frame;
    top_frame->recalc();
    params_group->recalc();
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
        recipe->ms_setup = nullptr;
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
        selfp = &ms->iparameters->at(index);
    } else {
        FXint index = cparams_listbox->getCurrentItem();
        /* Get the selected fit parameter. */
        selfp = &ms->cparameters->at(index);
    }
    /* Find the index of the fit parameter in the list of all
       possible parameters. */
    int fp_index = fit_parameters_find(param_list, selfp);
    /* Select in the listbox the given parameter. */
    listbox_select_parameter(param_listbox, fp_index);
    return 1;
}

void recipe_window::bind_new_acquisition(const struct acquisition_parameters *acquisition) {
    known_acquisition = *acquisition;
    setup_parameters_list();
}
