
/* regress_pro_window.h
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

#include <fx.h>
#include <sys/time.h>

#include "ProgressInfo.h"

#include "str.h"

#include "regress_pro.h"
#include "plot_canvas.h"
#include "spectra.h"
#include "fit_recipe.h"

class recipe_window;
class filmstack_window;
class dataset_window;
class batch_window;

class regress_pro_window : public FXMainWindow {
    FXDECLARE(regress_pro_window)

protected:
    struct spectrum *spectrum;
    struct stack *stack_result;
    fit_recipe *recipe;

    FXString recipeFilename;
    FXString spectrFile;

protected:
    filmstack_window *my_filmstack_window;
    recipe_window *my_recipe_window;
    dataset_window *my_dataset_window;
    batch_window *my_batch_window;
    filmstack_window *result_filmstack_window;

    FXMenuBar         *menubar;
    FXStatusBar       *statusbar;

    FXMenuPane        *filemenu;
    FXMenuPane        *editmenu;
    FXMenuPane        *spectrmenu;
    FXMenuPane        *dispmenu;
    FXMenuPane        *fitmenu;
    FXMenuPane        *helpmenu;

    FXTabBook         *tabbook;
    FXTabItem         *tabplot;
    FXText            *resulttext;

    FXFont            *scriptfont;

    static const FXchar patterns_recipe[];
    static const FXchar patterns_spectr[];

    static const FXHiliteStyle tstyles[];

protected:
    regress_pro_window() {};
private:
    regress_pro_window(const regress_pro_window&);
    regress_pro_window &operator=(const regress_pro_window&);
public:
    regress_pro* regressProApp() const { return (regress_pro*) getApp(); }

    fit_recipe *get_current_recipe() const { return recipe; }

    long onCmdFilmStack(FXObject*,FXSelector,void*);
    long onCmdRecipeEdit(FXObject*,FXSelector,void*);
    long onCmdDatasetEdit(FXObject*,FXSelector,void*);
    long onCmdLoadSpectra(FXObject*,FXSelector,void*);
    long onCmdSaveRecipe(FXObject*,FXSelector,void*);
    long onCmdLoadRecipe(FXObject*,FXSelector,void*);
    long onCmdPlotDispers(FXObject*,FXSelector,void*);
    long onCmdDispersOptim(FXObject*,FXSelector,void*);
    long onCmdRunFit(FXObject*,FXSelector,void*);
    long onCmdInteractiveFit(FXObject*,FXSelector,void*);
    long onCmdRunMultiFit(FXObject*,FXSelector,void*);
    long onCmdRunSimul(FXObject*,FXSelector,void*);
    long onCmdRunBatch(FXObject*,FXSelector,void*);
    long onCmdAbout(FXObject*,FXSelector,void*);
    long onCmdRegister(FXObject*,FXSelector,void*);
    long onCmdStackChange(FXObject*,FXSelector,void*);
    long onCmdStackShift(FXObject*,FXSelector,void*);
    long onCmdEditFilmStackResult(FXObject*,FXSelector,void*);
    long onUpdate(FXObject*,FXSelector,void*);

    void reportErrors();

public:
    enum {
        ID_SAVE_RECIPE = FXMainWindow::ID_LAST,
        ID_LOAD_RECIPE,
        ID_LOAD_SPECTRA,
        ID_DISP_PLOT,
        ID_DISP_OPTIM,
        ID_RUN_FIT,
        ID_INTERACTIVE_FIT,
        ID_RUN_MULTI_FIT,
        ID_RUN_BATCH,
        ID_RUN_SIMUL,
        ID_ABOUT,
        ID_SCRIPT_TEXT,
        ID_REGISTER,
        ID_FILM_STACK,
        ID_EDIT_FILMSTACK_RESULT,
        ID_RECIPE_EDIT,
        ID_DATASET_EDIT,
        ID_STACK_CHANGE,
        ID_STACK_SHIFT,
        ID_LAST
    };

public:
    regress_pro_window(regress_pro *a);
    virtual void create();
    virtual ~regress_pro_window();

private:
    bool check_spectrum(const char *context);
    void run_fit(fit_engine *fit, seeds *fseeds, struct spectrum *fspectrum);

    struct spectrum* m_model_spectr;

    plot_canvas* m_canvas;

    bool m_title_dirty;
};

extern "C" {

    extern int process_foxgui_events(void *data, float p, const char *msg);

};
