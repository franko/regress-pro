#include "batch_window.h"
#include "grid-search.h"
#include "regress_pro_window.h"
#include "error-messages.h"

extern "C" {
    static int window_process_events(void *data, float p, const char *msg);
};

// Map
FXDEFMAP(batch_window) batch_window_map[]= {
    FXMAPFUNC(SEL_COMMAND, batch_window::ID_RUN_BATCH, batch_window::on_cmd_run_batch),
    FXMAPFUNC(SEL_COMMAND, batch_window::ID_DELETE, batch_window::onCmdHide),
};

FXIMPLEMENT(batch_window,FXDialogBox,batch_window_map,ARRAYNUMBER(batch_window_map));

batch_window::batch_window(regress_pro_window *w, FXuint opts, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(w, "Spectra Dataset", opts, 0, 0, 540, 400, pl, pr, pt, pb, hs, vs),
    app_window(w)
{
    FXHorizontalFrame *hframe = new FXHorizontalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    table = new filelist_table(hframe, nullptr, 0, TABLE_COL_SIZABLE|TABLE_ROW_SIZABLE|LAYOUT_FILL_X|LAYOUT_FILL_Y);

    FXVerticalFrame *bframe = new FXVerticalFrame(hframe, LAYOUT_FILL_Y);
    new FXButton(bframe, "Add Files", nullptr, table, filelist_table::ID_ADD_FILES);
    new FXButton(bframe, "Remove all", nullptr, table, filelist_table::ID_REMOVE_FILES);
    new FXButton(bframe, "Run", nullptr, this, ID_RUN_BATCH);
}

int batch_window::batch_run(fit_recipe *recipe, str_ptr *error_msg)
{
    if (recipe->parameters->number == 0 ||
        check_fit_parameters(recipe->stack, recipe->parameters, error_msg) != 0) {
        return 1;
    }

    int missing_columns = recipe->parameters->number + 2 - table->getNumColumns();
    if (missing_columns > 0) {
        table->insertColumns(table->getNumColumns(), missing_columns);
    }

    str_t pname;
    str_init(pname, 15);
    int j;
    for (j = 0; j < int(recipe->parameters->number); j++) {
        get_param_name(&recipe->parameters->values[j], pname);
        table->setColumnText(j + 1, CSTR(pname));
    }
    table->setColumnText(j + 1, "Chi Square");
    str_free(pname);
    for (j++; j + 1 < table->getNumColumns(); j++) {
        table->setColumnText(j + 1, "");
    }

    table->removeRange(0, table->samples_number() - 1, 1, table->getNumColumns() - 1);

    fit_engine *fit = fit_engine_new();
    fit_engine_bind(fit, recipe->stack, recipe->config, recipe->parameters);

    FXString result;
    for (int i = 0; i < table->samples_number(); i++) {
        FXString name = table->getItemText(i, 0);
        spectrum *s = load_gener_spectrum(name.text(), error_msg);
        if (!s || fit_engine_prepare_check_error(fit, s) != nullptr) {
            return 1;
        }
        fit_engine_prepare(fit, s, FIT_RESET_ACQUISITION);
        lmfit_result fresult;
        lmfit_grid(fit, recipe->seeds_list, &fresult, nullptr, LMFIT_PRESERVE_STACK,
                   window_process_events, this);

        unsigned j;
        for (j = 0; j < recipe->parameters->number; j++) {
            result.format("%g", gsl_vector_get(fit->run->results, j));
            table->setItemText(i, j + 1, result);
        }
        result.format("%g", fresult.chisq);
        table->setItemText(i, j + 1, result);

        fit_engine_disable(fit);
        spectra_free(s);
    }

    fit_engine_free(fit);
    return 0;
}

long batch_window::on_cmd_run_batch(FXObject *, FXSelector, void *)
{
    fit_recipe *recipe = app_window->get_current_recipe();
    if (recipe) {
        str_ptr error_msg;
        if (batch_run(recipe, &error_msg)) {
            FXMessageBox::information(this, MBOX_OK, "Running batch", "%s\n", CSTR(error_msg));
            free_error_message(error_msg);
        }
    }
    return 1;
}

int
window_process_events(void *data, float p, const char *msg)
{
    FXWindow *win = (FXWindow *) data;
    FXApp *app = win->getApp();
    app->runModalWhileEvents(win);
    return 0;
}
