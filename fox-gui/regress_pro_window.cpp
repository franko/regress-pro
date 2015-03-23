
/* regress_pro_window.cpp
 *
 * Copyright (C) 2005-2011 Francesco Abbate
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include <gsl/gsl_vector.h>

#include "regress_pro_window.h"
#include "dispers_dialog.h"
#include "BatchDialog.h"
#include "interactive_fit.h"
#include "Strcpp.h"
#include "error-messages.h"
#include "fit-engine.h"
#include "multi-fit-engine.h"
#include "lmfit-multi.h"
#include "fit-params.h"
#include "spectra.h"
#include "symtab.h"
#include "grid-search.h"
#include "str.h"
#include "str-util.h"
#include "dispers-library.h"
#include "disp_chooser.h"
#include "disp_fit_manager.h"
#include "fit_window.h"
#include "disp_fit_window.h"
#include "interactive_fit.h"
#include "recipe_window.h"
#include "filmstack_window.h"
#include "dataset_window.h"
#include "batch_window.h"
#include "lexer.h"

static float timeval_subtract(struct timeval *x, struct timeval *y);

#ifdef WIN32
#define SCRIPT_FONT_NAME "Courier New"
#else
#define SCRIPT_FONT_NAME "Monospace"
#endif

// Map
FXDEFMAP(regress_pro_window) regress_pro_window_map[]= {
    FXMAPFUNC(SEL_UPDATE,  0, regress_pro_window::onUpdate),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_ABOUT,  regress_pro_window::onCmdAbout),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_REGISTER,  regress_pro_window::onCmdRegister),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_FILM_STACK, regress_pro_window::onCmdFilmStack),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_RECIPE_EDIT, regress_pro_window::onCmdRecipeEdit),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_DATASET_EDIT, regress_pro_window::onCmdDatasetEdit),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_LOAD_SCRIPT, regress_pro_window::onCmdLoadScript),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_SAVE_SCRIPT, regress_pro_window::onCmdSaveScript),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_SAVEAS_SCRIPT, regress_pro_window::onCmdSaveAsScript),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_SAVE_RECIPE, regress_pro_window::onCmdSaveRecipe),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_LOAD_RECIPE, regress_pro_window::onCmdLoadRecipe),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_LOAD_SPECTRA, regress_pro_window::onCmdLoadSpectra),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_DISP_PLOT, regress_pro_window::onCmdPlotDispers),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_DISP_OPTIM, regress_pro_window::onCmdDispersOptim),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_RUN_FIT, regress_pro_window::onCmdRunFit),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_RUN_FIT_NEW, regress_pro_window::onCmdRunFitNew),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_INTERACTIVE_FIT, regress_pro_window::onCmdInteractiveFit),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_RUN_MULTI_FIT, regress_pro_window::onCmdRunMultiFit),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_RUN_MULTI_FIT_NEW, regress_pro_window::onCmdRunMultiFitNew),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_RUN_BATCH, regress_pro_window::onCmdRunBatch),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_RUN_BATCH_NEW, regress_pro_window::onCmdRunBatchNew),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_STACK_CHANGE, regress_pro_window::onCmdStackChange),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_STACK_SHIFT, regress_pro_window::onCmdStackShift),
    FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_EDIT_FILMSTACK_RESULT, regress_pro_window::onCmdEditFilmStackResult),
};


// Object implementation
FXIMPLEMENT(regress_pro_window,FXMainWindow,regress_pro_window_map,ARRAYNUMBER(regress_pro_window_map));


const FXchar regress_pro_window::patterns_fit[] =
    "Fit Strategy (*.fit)"
    "\nAll Files (*)";
const FXchar regress_pro_window::patterns_spectr[] =
    "Fit Strategy (*.dat)"
    "\nAll Files (*)";
const FXchar regress_pro_window::patterns_recipe[] =
    "Fit Recipe (*.rcp)"
    "\nAll Files (*)";

const FXHiliteStyle regress_pro_window::tstyles[] = {
    {FXRGB(255,255,255), FXRGB(255,0,0), 0, 0, 0, 0, 0, 0}
};



// Make some windows
regress_pro_window::regress_pro_window(elliss_app* a)
    : FXMainWindow(a,"Regress Pro",NULL,&a->appicon,DECOR_ALL,20,20,700,460),
      spectrum(NULL), stack_result(NULL), scriptFile("untitled"),
      spectrFile("untitled"), batchFileId("untitled####.dat"),
      my_filmstack_window(NULL), my_recipe_window(NULL),
      my_dataset_window(NULL), my_batch_window(NULL), result_filmstack_window(NULL), m_model_spectr(0)
{
    // Menubar
    menubar=new FXMenuBar(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
    statusbar=new FXStatusBar(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|FRAME_RAISED|STATUSBAR_WITH_DRAGCORNER);

    // Script menu
    filemenu=new FXMenuPane(this);
    new FXMenuCommand(filemenu,"&Load",NULL,this,ID_LOAD_SCRIPT);
    new FXMenuCommand(filemenu,"&Save",NULL,this,ID_SAVE_SCRIPT);
    new FXMenuCommand(filemenu,"Save As",NULL,this,ID_SAVEAS_SCRIPT);
    new FXMenuCommand(filemenu,"Save Recipe",NULL,this,ID_SAVE_RECIPE);
    new FXMenuCommand(filemenu,"Load Recipe",NULL,this,ID_LOAD_RECIPE);
    new FXMenuCommand(filemenu,"&Quit\tCtl-Q",NULL,getApp(),FXApp::ID_QUIT);
    new FXMenuTitle(menubar,"&Script",NULL,filemenu);

    // Edit menu
    editmenu = new FXMenuPane(this);
    new FXMenuCommand(editmenu, "Film Stack", NULL, this, ID_FILM_STACK);
    new FXMenuCommand(editmenu, "Recipe", NULL, this, ID_RECIPE_EDIT);
    new FXMenuCommand(editmenu, "Dataset", NULL, this, ID_DATASET_EDIT);
    new FXMenuTitle(menubar, "&Edit", NULL, editmenu);

    // Script menu
    spectrmenu = new FXMenuPane(this);
    new FXMenuCommand(spectrmenu,"&Load Spectra",NULL,this,ID_LOAD_SPECTRA);
    new FXMenuTitle(menubar,"S&pectra",NULL,spectrmenu);

    // Dispersion menu
    dispmenu = new FXMenuPane(this);
    new FXMenuCommand(dispmenu, "&Plot Dispersion",NULL,this,ID_DISP_PLOT);
    new FXMenuCommand(dispmenu, "Dispersion Optimize",NULL,this,ID_DISP_OPTIM);
    new FXMenuTitle(menubar,"&Dispersion",NULL,dispmenu);

    // Fit menu
    fitmenu = new FXMenuPane(this);
    new FXMenuCommand(fitmenu, "Run (NEW) Fitting",NULL,this,ID_RUN_FIT_NEW);
    new FXMenuCommand(fitmenu, "Edit Result Stack",NULL,this,ID_EDIT_FILMSTACK_RESULT);
    new FXMenuCommand(fitmenu, "&Run Fitting",NULL,this,ID_RUN_FIT);
    new FXMenuCommand(fitmenu, "&Interactive Fit",NULL,this,ID_INTERACTIVE_FIT);
    new FXMenuCommand(fitmenu, "Run &Multiple Fit",NULL,this,ID_RUN_MULTI_FIT);
    new FXMenuCommand(fitmenu, "Run (NEW) Multiple Fit",NULL,this,ID_RUN_MULTI_FIT_NEW);
    new FXMenuCommand(fitmenu, "Run &Batch",NULL,this,ID_RUN_BATCH);
    new FXMenuCommand(fitmenu, "Run Batch (NEW)",NULL,this,ID_RUN_BATCH_NEW);
    new FXMenuTitle(menubar,"Fittin&g",NULL,fitmenu);

    helpmenu = new FXMenuPane(this);
    new FXMenuCommand(helpmenu, "&Register", NULL, this, ID_REGISTER);
    new FXMenuCommand(helpmenu, "&About", NULL, this, ID_ABOUT);
    new FXMenuTitle(menubar, "&Help", NULL, helpmenu, LAYOUT_RIGHT);

    // Container
    FXHorizontalFrame *cont = new FXHorizontalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);

    tabbook = new FXTabBook(cont,NULL,0,PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_RIGHT);

    // First item is a list
    tabscript = new FXTabItem(tabbook,"Script",NULL);
    FXHorizontalFrame *lf = new FXHorizontalFrame(tabbook,FRAME_THICK|FRAME_RAISED);
    FXHorizontalFrame *bf = new FXHorizontalFrame(lf,FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);
    scripttext = new FXText(bf,this,ID_SCRIPT_TEXT,TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    scripttext->setStyled(TRUE);
    scripttext->setHiliteStyles(tstyles);
    scriptfont = new FXFont(getApp(), SCRIPT_FONT_NAME, 10);
    scripttext->setFont(scriptfont);

    new FXTabItem(tabbook,"Fit Results",NULL);
    lf = new FXHorizontalFrame(tabbook,FRAME_THICK|FRAME_RAISED);
    bf = new FXHorizontalFrame(lf,FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);
    resulttext = new FXText(bf,NULL,0,TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    resulttext->setFont(scriptfont);

    tabplot = new FXTabItem(tabbook,"Plot Result",NULL);
    lf = new FXHorizontalFrame(tabbook,FRAME_THICK|FRAME_RAISED);
    bf = new FXHorizontalFrame(lf,FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);

    symbol_table_init(this->symtab);

    init_class_list();

    dispers_library_init();

    recipe = new fit_recipe();
    recipe->setup_default_stack();

    m_canvas = new plot_canvas(bf, NULL, 0, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    m_title_dirty = true;
    m_title_modified = false;
}


void
regress_pro_window::create()
{
    FXMainWindow::create();
    scriptfont->create();
}

long
regress_pro_window::onUpdate(FXObject* sender, FXSelector sel, void* ptr)
{
    FXMainWindow::onUpdate(sender, sel, ptr);

    bool is_mod = scripttext->isModified();

    if(m_title_dirty || (is_mod != m_title_modified)) {
        bool is_reg = get_elliss_app()->is_registered();

        FXString filename = scriptFile.rafter(DIR_SEPARATOR);
        FXString pathname = scriptFile.rbefore(DIR_SEPARATOR);
        FXString flag(is_mod ? "*" : "");
        FXString appname(is_reg ? "Regress Pro" : "(UNREGISTERED)");

        this->setTitle(flag + filename + " - " + pathname + " - " + appname);

        m_title_dirty = false;
        m_title_modified = is_mod;

        return 1;
    }

    return 0;
}

long
regress_pro_window::onCmdLoadScript(FXObject*,FXSelector,void *)
{
    reg_check_point(this);

    FXFileDialog open(this,"Open Script");
    open.setFilename(scriptFile);
    open.setPatternList(patterns_fit);

    if(open.execute()) {
        scriptFile = open.getFilename();
        Str script_text;

        if(str_loadfile(scriptFile.text(), script_text.str()) != 0) {
            return 0;
        }

#ifdef ENABLE_SCRIPT_REL_PATH
        str_init_from_c(script_file, scriptFile.text());
        str_dirname(this->symtab->env->script_dir, script_file, DIR_SEPARATOR);
        str_free(script_file);
#endif

        scripttext->setText(script_text.cstr());
        set_fit_strategy(script_text.cstr());
        scripttext->setModified(FALSE);

        m_title_dirty = true;
    }

    return 1;
}

bool
regress_pro_window::save_script_as(const FXString& save_as)
{
    FILE *f = fopen(save_as.text(), "w");

    if(f == NULL) {
        FXMessageBox::information(this, MBOX_OK, "Script save",
                                  "Cannot write file %s\n", scriptFile.text());
        return false;
    }

    if(fputs(scripttext->getText().text(), f) == EOF) {
        FXMessageBox::information(this, MBOX_OK, "Script save",
                                  "Cannot write file %s\n", scriptFile.text());
        fclose(f);
        return false;
    }

    fputc('\n', f);
    fclose(f);

    update_fit_strategy();

    scripttext->setModified(FALSE);

    m_title_dirty = true;

    return true;
}

long
regress_pro_window::onCmdSaveAsScript(FXObject*,FXSelector,void *)
{
    reg_check_point(this);

    FXFileDialog open(this, "Save Script As");
    open.setFilename(scriptFile);
    open.setPatternList(patterns_fit);

    if(open.execute()) {
        FXString new_filename = open.getFilename();
        if(save_script_as(new_filename)) {
            scriptFile = new_filename;
        }
        return 1;
    }

    return 0;
}

long
regress_pro_window::onCmdSaveScript(FXObject*,FXSelector,void *)
{
    reg_check_point(this);

    save_script_as(scriptFile);
    return 1;
}

long
regress_pro_window::onCmdLoadSpectra(FXObject*,FXSelector,void *)
{
    reg_check_point(this);

    FXFileDialog open(this,"Open Spectra");
    open.setFilename(spectrFile);
    open.setPatternList(patterns_spectr);

    if(open.execute()) {
        spectrFile = open.getFilename();

        struct spectrum *new_spectrum = load_gener_spectrum(spectrFile.text());

        if(new_spectrum == NULL) {
            FXMessageBox::information(this, MBOX_OK, "Spectra loading",
                                      "Cannot load spectra %s", spectrFile.text());
        } else {
            m_canvas->clear_plots();

            if(this->spectrum) {
                spectra_free(this->spectrum);
            }

            this->spectrum = new_spectrum;

            spectra_plot_simple(m_canvas, this->spectrum);
        }

        return 1;
    }

    return 0;
}

long
regress_pro_window::onCmdPlotDispers(FXObject*,FXSelector,void*)
{
    if(this->stack_result) {
        dispers_dialog dialog(this, this->stack_result);
        dialog.execute();
        return 1;
    }
    return 0;
}

long
regress_pro_window::onCmdDispersOptim(FXObject*,FXSelector,void*)
{
    reg_check_point(this);

    struct disp_fit_engine *fit = disp_fit_engine_new();

    fit->ref_disp = dispers_library_search("sio2");
    fit->model_disp = dispers_library_search("sio2");

    disp_fit_manager *mgr = new disp_fit_manager(fit);
    disp_fit_window *fitwin = new disp_fit_window(mgr, getApp(), "Dispersion Fit", NULL, NULL, DECOR_ALL, 0, 0, 640, 480);
    fitwin->create();
    fitwin->show(FX::PLACEMENT_SCREEN);

    return 1;
}

long
regress_pro_window::onCmdAbout(FXObject *, FXSelector, void *)
{
    FXDialogBox about(this,"About Regress Pro",DECOR_TITLE|DECOR_BORDER,0,0,0,0,
                      0,0,0,0, 0,0);
    FXVerticalFrame* side=new FXVerticalFrame(&about,LAYOUT_SIDE_RIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 10,10,10,10, 0,0);
    new FXLabel(side,"R e g r e s s   P r o",NULL,JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_FILL_X);
    new FXHorizontalSeparator(side,SEPARATOR_LINE|LAYOUT_FILL_X);
    new FXLabel(side,FXStringFormat("\nRegress Pro, version %d.%d.%d.\n\n" "Regress Pro is a scientific / industrial software to perform regression\nanalysis of measurement data coming from spectroscopic\nellipsometers or reflectometers.\n" "Regress Pro uses the FOX Toolkit version %d.%d.%d.\nCopyright (C) 2005-2011 Francesco Abbate (francesco.bbt@gmail.com).\n",VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH,FOX_MAJOR,FOX_MINOR,FOX_LEVEL),NULL,JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
    FXButton *button=new FXButton(side,"&OK",NULL,&about,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,32,32,2,2);
    button->setFocus();
    about.execute(PLACEMENT_OWNER);
    return 1;
}

long
regress_pro_window::onCmdRegister(FXObject *, FXSelector, void *)
{
    reg_form(this);
    m_title_dirty = true;
    return 1;
}

// Clean up
regress_pro_window::~regress_pro_window()
{
    delete scriptfont;
    delete filemenu;
    delete editmenu;
    delete spectrmenu;
    delete dispmenu;
    delete fitmenu;
    delete helpmenu;

    delete recipe;

    if(this->spectrum) {
        spectra_free(this->spectrum);
    }

    if(this->stack_result) {
        stack_free(this->stack_result);
    }

    if(m_model_spectr) {
        spectra_free(m_model_spectr);
    }

    symbol_table_clean(this->symtab);
    symbol_table_free(this->symtab);

    clean_error_msgs();
}

long
regress_pro_window::onCmdRunBatch(FXObject*,FXSelector,void *)
{
    struct seeds *seeds;
    struct fit_engine *fit;

    reg_check_point(this);

    fit = build_fit_engine(this->symtab, &seeds);

    if(fit == NULL) {
        return 0;
    }

    BatchDialog batch(this, fit, seeds);
    batch.setFilename(batchFileId);

    FXString result;
    batch.execute(result);
    resulttext->setText(result);
    resulttext->setModified(TRUE);

    batchFileId = batch.getFilename();

    fit_engine_free(fit);

    return 1;
}

long
regress_pro_window::onCmdRunBatchNew(FXObject *, FXSelector, void *)
{
    reg_check_point(this);
    if (!my_batch_window) {
        my_batch_window = new batch_window(this);
        my_batch_window->create();
    }
    my_batch_window->show(PLACEMENT_SCREEN);
    return 1;
}

long
regress_pro_window::onCmdRunMultiFitNew(FXObject*,FXSelector,void *)
{
    if (!my_dataset_window || !my_recipe_window || !recipe->ms_setup) {
        return 0;
    }

    struct multi_fit_engine *fit;
    const fit_parameters *iparams = recipe->ms_setup->iparameters;
    const fit_parameters *cparams = recipe->ms_setup->cparameters;
    dataset_table *dataset = my_dataset_window->dataset();
    int samples_number = dataset->samples_number();
    fit = multi_fit_engine_new(recipe->config, samples_number);
    multi_fit_engine_bind(fit, recipe->stack, recipe->parameters, iparams);
    double *constr_values = new double[cparams->number];
    for (int i = 0; i < samples_number; i++) {
        int error_col;
        bool ok = dataset->get_values(i, cparams, constr_values, error_col);
        if (!ok) {
            FXMessageBox::information(this, MBOX_OK, "Multiple Fit error", "Missing parameter");
            delete [] constr_values;
            multi_fit_engine_free(fit);
            return 1;
        }
        multi_fit_engine_apply_parameters(fit, cparams, constr_values);
    }
    delete [] constr_values;

    FXString error_filename;
    bool ok = dataset->get_spectra_list(fit->spectra_list, error_filename);
    if (!ok) {
        FXMessageBox::information(this, MBOX_OK, "Multiple Fit error", "error opening: %s", error_filename.text());
        multi_fit_engine_free(fit);
        return 1;
    }

    if(multi_fit_engine_prepare(fit) != 0) {
        FXMessageBox::information(this, MBOX_OK, "Multiple Fit error", "Error preparing multi fit calculatin");
        multi_fit_engine_free(fit);
        return 1;
    }

    struct seeds *iseeds = seed_list_new();

    double *iseed_values = new double[iparams->number];
    for (int i = 0; i < samples_number; i++) {
        int error_col;
        bool ok = dataset->get_values(i, iparams, iseed_values, error_col);
        if (!ok) {
            FXMessageBox::information(this, MBOX_OK, "Multiple Fit error", "Missing parameter");
            delete [] iseed_values;
            seed_list_free(iseeds);
            multi_fit_engine_free(fit);
            return 1;
        }
        for (unsigned j = 0; j < iparams->number; j++) {
            seed_list_add_simple(iseeds, iseed_values[j]);
        }
    }
    delete [] iseed_values;

    Str fit_error_msgs;
    ProgressInfo progress(this->getApp(), this);

    Str analysis;
    lmfit_multi(fit, recipe->seeds_list, iseeds,
                analysis.str(), fit_error_msgs.str(),
                LMFIT_GET_RESULTING_STACK,
                process_foxgui_events, &progress);

    progress.hide();

    if(fit_error_msgs.length() > 0) {
        FXMessageBox::information(this, MBOX_OK, "Multiple Fit messages", "%s", fit_error_msgs.cstr());
        clean_error_msgs();
    }

    FXString text_fit_result;

    Str fp_results;
    multi_fit_engine_print_fit_results(fit, fp_results.str());
    text_fit_result.append(fp_results.cstr());

    text_fit_result.append(analysis.cstr());

    resulttext->setText(text_fit_result);
    resulttext->setModified(TRUE);

    seed_list_free(iseeds);
    multi_fit_engine_disable(fit);
    multi_fit_engine_free(fit);

    return 1;
}

long
regress_pro_window::onCmdRunMultiFit(FXObject*,FXSelector,void *)
{
    struct multi_fit_engine *fit;
    Str analysis;
    struct {
        struct seeds *common;
        struct seeds *individual;
    } seeds;

    if(! update_fit_strategy()) {
        return 1;
    }

    fit = build_multi_fit_engine(this->symtab, &seeds.common, &seeds.individual);

    if(fit == NULL) {
        return 1;
    }

    Str fit_error_msgs;
    ProgressInfo progress(this->getApp(), this);

    lmfit_multi(fit, seeds.common, seeds.individual,
                analysis.str(), fit_error_msgs.str(),
                LMFIT_GET_RESULTING_STACK,
                process_foxgui_events, &progress);

    progress.hide();

    if(fit_error_msgs.length() > 0) {
        FXMessageBox::information(this, MBOX_OK, "Multiple Fit messages",
                                  fit_error_msgs.cstr());
        clean_error_msgs();
    }

    FXString text_fit_result;

    Str fp_results;
    multi_fit_engine_print_fit_results(fit, fp_results.str());
    text_fit_result.append(fp_results.cstr());

    text_fit_result.append(analysis.cstr());

    resulttext->setText(text_fit_result);
    resulttext->setModified(TRUE);

    multi_fit_engine_disable(fit);
    multi_fit_engine_free(fit);

    return 1;
}

bool
regress_pro_window::update_fit_strategy()
{
    if(scripttext->isModified()) {
        return set_fit_strategy(scripttext->getText().text());
    }
    return true;
}

bool
regress_pro_window::check_spectrum(const char *context)
{
    if(spectrum == NULL) {
        FXMessageBox::information(this, MBOX_OK, context,
                                  "Please load a spectra before.");
        return false;
    }

    return true;
}

void
regress_pro_window::run_fit(fit_engine *fit, seeds *fseeds, struct spectrum *fspectrum)
{
    double chisq;
    Str analysis;

    fit_engine_prepare(fit, fspectrum);

    Str fit_error_msgs;
    ProgressInfo progress(this->getApp(), this);

    lmfit_grid(fit, fseeds, &chisq, analysis.str(), fit_error_msgs.str(),
               LMFIT_GET_RESULTING_STACK,
               process_foxgui_events, & progress);

    progress.hide();

    if(fit_error_msgs.length() > 0) {
        FXMessageBox::information(this, MBOX_OK, "Fit messages",
                                  fit_error_msgs.cstr());
        clean_error_msgs();
    }

    FXString fitresult, row;

    /* name of the fit script */
    row.format("%s :\n", scriptFile.text());
    fitresult.append(row);

    /* fit parameters results */
    Str fit_parameters_results;
    fit_engine_print_fit_results(fit, fit_parameters_results.str(), 0);
    fitresult.append(fit_parameters_results.cstr());

    /* final chi square obtained */
    row.format("Residual Chisq/pt: %g\n", chisq);
    fitresult.append(row);

    /* covariance matrix analysis */
    fitresult.append("\n");
    fitresult.append(analysis.cstr());

    resulttext->setText(fitresult);
    resulttext->setModified(TRUE);

    if(m_model_spectr) {
        spectra_free(m_model_spectr);
    }

    m_model_spectr = spectra_alloc(fspectrum);

    fit_engine_generate_spectrum(fit, fspectrum, m_model_spectr);

    spectra_plot(m_canvas, fspectrum, m_model_spectr);

    if(this->stack_result) {
        stack_free(this->stack_result);
    }

    this->stack_result = fit_engine_yield_stack(fit);
    if (result_filmstack_window) {
        result_filmstack_window->bind_new_filmstack(this->stack_result);
    }

    fit_engine_disable(fit);
}

long
regress_pro_window::onCmdRunFit(FXObject*,FXSelector,void *)
{
    if(! check_spectrum("Fitting")) {
        return 0;
    }

    reg_check_point(this);

    if(! update_fit_strategy()) {
        return 1;
    }

    struct seeds *seeds;
    struct fit_engine *fit;

    fit = build_fit_engine(this->symtab, &seeds);

    if(fit == NULL) {
        reportErrors();
        return 1;
    }

    run_fit(fit, seeds, this->spectrum);
    fit_engine_free(fit);

    getApp()->endWaitCursor();

    return 1;
}

long
regress_pro_window::onCmdRunFitNew(FXObject*,FXSelector,void *)
{
    if(! check_spectrum("Fitting")) {
        return 0;
    }
    reg_check_point(this);
    if (recipe->parameters->number == 0 ||
        check_fit_parameters(recipe->stack, recipe->parameters) != 0) {
        // ADD a proper ERROR MESSAGE here.
        return 0;
    }
    fit_engine *fit = fit_engine_new();
    fit_engine_bind(fit, recipe->stack, recipe->config, recipe->parameters);
    run_fit(fit, recipe->seeds_list, this->spectrum);
    fit_engine_free(fit);
    getApp()->endWaitCursor();
    return 1;
}

long
regress_pro_window::onCmdInteractiveFit(FXObject*,FXSelector,void*)
{
    if(! check_spectrum("Fitting")) {
        return 0;
    }

    reg_check_point(this);

    if(! update_fit_strategy()) {
        return 1;
    }

    struct seeds *seeds;
    struct fit_engine *fit = build_fit_engine(symtab, &seeds);

    if(fit == NULL) {
        reportErrors();
        return 1;
    }

    interactive_fit *fitmgr = new interactive_fit(fit, spectrum);
    elliss_app *app = get_elliss_app();
    fit_window *fitwin = new fit_window(fitmgr, app, "Interactive Fit", NULL, &app->appicon, DECOR_ALL, 0, 0, 640, 480);
    fitwin->create();
    fitwin->show(FX::PLACEMENT_SCREEN);
    return 1;
}

long
regress_pro_window::onCmdFilmStack(FXObject*,FXSelector,void*)
{
    if (!my_filmstack_window) {
        my_filmstack_window = new filmstack_window(recipe->stack, "Fit Stack", this);
        my_filmstack_window->set_target_stack_changes(this, FXSEL(SEL_COMMAND,ID_STACK_CHANGE), FXSEL(SEL_COMMAND,ID_STACK_SHIFT));
        my_filmstack_window->create();
    }
    my_filmstack_window->show(PLACEMENT_SCREEN);
    return 1;
}

long
regress_pro_window::onCmdEditFilmStackResult(FXObject*,FXSelector,void*)
{
    if (!stack_result) {
        return 0;
    }
    if (!result_filmstack_window) {
        result_filmstack_window = new filmstack_window(stack_result, "Result Stack", this);
        result_filmstack_window->create();
    }
    result_filmstack_window->show(PLACEMENT_SCREEN);
    return 1;
}

long
regress_pro_window::onCmdRecipeEdit(FXObject*,FXSelector,void*)
{
    if (!my_recipe_window) {
        recipe_window *w = new recipe_window(recipe, this);
        my_recipe_window = w;
        w->create();
    }
    my_recipe_window->show(PLACEMENT_SCREEN);
    return 1;
}

long
regress_pro_window::onCmdDatasetEdit(FXObject*,FXSelector,void*)
{
    if (!my_dataset_window) {
        my_dataset_window = new dataset_window(recipe, this);
        my_dataset_window->create();
    }
    my_dataset_window->show(PLACEMENT_SCREEN);
    return 1;
}

long
regress_pro_window::onCmdStackChange(FXObject*,FXSelector,void*)
{
    if (my_recipe_window) {
        my_recipe_window->handle(this, FXSEL(SEL_COMMAND, recipe_window::ID_STACK_CHANGE), NULL);
    }
    if (my_dataset_window) {
        dataset_table *dataset = my_dataset_window->dataset();
        dataset->handle(this, FXSEL(SEL_COMMAND, dataset_table::ID_STACK_CHANGE), recipe->stack);
    }
    return 1;
}

long
regress_pro_window::onCmdStackShift(FXObject *, FXSelector, void *ptr)
{
    recipe->shift_fit_parameters((shift_info *)ptr);
    if (my_dataset_window) {
        dataset_table *dataset = my_dataset_window->dataset();
        dataset->handle(this, FXSEL(SEL_COMMAND, dataset_table::ID_STACK_SHIFT), (shift_info *)ptr);
    }
    return 1;
}

bool
regress_pro_window::set_fit_strategy(const char *script_text)
{
    cleanScriptErrors();

    if(parse_strategy(this->symtab, script_text) != 0) {
        int fline, lline;
        str_t errmsg;
        str_init(errmsg, 128);

        if(get_script_error_region(&fline, &lline) == 0) {
            setErrorRegion(fline, lline);
        }

        get_errors_list(errmsg);

        FXMessageBox::information(this, MBOX_OK, "Script parsing",
                                  "The parsing of the script has been"
                                  " unsuccessful :\n%s", CSTR(errmsg));

        str_free(errmsg);
        return false;
    }

    return true;
}

void
regress_pro_window::reportErrors()
{
    str_t errmsg;
    str_init(errmsg, 128);
    get_errors_list(errmsg);
    FXMessageBox::information(this, MBOX_OK, "Script parsing",
                              "The parsing of the script has been"
                              " unsuccessful :\n%s", CSTR(errmsg));
    str_free(errmsg);
}

void
regress_pro_window::cleanScriptErrors()
{
    scripttext->changeStyle(0, scripttext->getLength(), 0);
}

void
regress_pro_window::setErrorRegion(int sl, int el)
{
    int cl = 1, ns = 0, next;
    FXString text(scripttext->getText());
    const char *base = text.text();

    while(cl < sl) {
        const char *ptr = strchr(base, '\n');
        if(! ptr) {
            break;
        }
        ptr ++;
        ns += ptr - base;
        base = ptr;
        cl ++;
    }

    if(cl < sl) {
        return;
    }

    next = 0;
    while(cl <= el) {
        const char *ptr = strchr(base, '\n');
        if(! ptr) {
            break;
        }
        ptr ++;
        next += ptr - base;
        base = ptr;
        cl ++;
    }

    scripttext->changeStyle(ns, next, 1);
}

float
timeval_subtract(struct timeval *x, struct timeval *y)
{
    float result = y->tv_sec - x->tv_sec;
    result += (y->tv_usec - x->tv_usec) / 1.0E6;
    return result;
}

int
process_foxgui_events(void *data, float progr, const char *msg)
{
    const float wait_time = 0.4, progress_thresold = 0.5;
    ProgressInfo *info = (ProgressInfo *) data;
    struct timeval current[1];

    if(gettimeofday(current, NULL) != 0) {
        return 0;
    }

    float diff = timeval_subtract(info->start, current);
    const int multfactor = 4096;

    if(info->dialog == NULL) {
        if(diff < wait_time || progr > progress_thresold) {
            return 0;
        }

        info->dialog = new FXProgressDialog(info->window,
                                            "Fit is running",
                                            "Please wait...",
                                            PROGRESSDIALOG_CANCEL);

        info->dialog->setTotal(multfactor);
        info->dialog->setBarStyle(PROGRESSBAR_PERCENTAGE);
        info->dialog->create();
        info->dialog->show(PLACEMENT_SCREEN);

        info->dialog->setProgress((int)(progr * multfactor));
        info->dialog->repaint();

        info->app->beginWaitCursor();

        return 0;
    }

    if(info->dialog) {
        info->dialog->setProgress((int)(progr * multfactor));
        info->dialog->repaint();
        if(msg) {
            info->dialog->setMessage(msg);
        }

        info->app->runModalWhileEvents(info->dialog);

        return (info->dialog->shown() ? 0 : 1);
    }

    return 0;
}

long
regress_pro_window::onCmdSaveRecipe(FXObject *, FXSelector, void *)
{
    FXFileDialog open(this, "Save Recipe As");
    open.setPatternList(patterns_recipe);

    if(open.execute()) {
        FXString new_filename = open.getFilename();
        writer_t *w = writer_new();
        recipe->write(w);
        FILE *f = fopen(new_filename.text(), "wb");
        if (f) {
            fputs(CSTR(w->text), f);
            fclose(f);
        }
        writer_free(w);
    }
    return 1;
}

long
regress_pro_window::onCmdLoadRecipe(FXObject *, FXSelector, void *)
{
    FXFileDialog open(this, "Open Fit Recipe");
    open.setPatternList(patterns_recipe);

    if(open.execute()) {
        FXString filename = open.getFilename();
        Str content;

        if(str_loadfile(filename.text(), content.str()) != 0) {
            return 1;
        }

        lexer_t *l = lexer_new(content.cstr());
        fit_recipe *new_recipe = fit_recipe::read(l);
        if (!new_recipe) {
            lexer_free(l);
            return 1;
        }
        lexer_free(l);
        fit_recipe *old_recipe = recipe;
        recipe = new_recipe;
        if (my_recipe_window) {
            my_recipe_window->bind_new_fit_recipe(recipe);
        }
        if (my_filmstack_window) {
            my_filmstack_window->bind_new_filmstack(recipe->stack, false);
        }
        if (my_dataset_window) {
            dataset_table *dataset = my_dataset_window->dataset();
            dataset->handle(this, FXSEL(SEL_COMMAND, dataset_table::ID_STACK_CHANGE), recipe->stack);
        }
        delete old_recipe;
    }
    return 1;
}
