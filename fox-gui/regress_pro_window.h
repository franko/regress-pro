
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
#include "fit_window.h"
#include "filmstack_window.h"

class recipe_window;
class dataset_window;
class batch_window;
class interactive_fit;
class disp_fit_window;

struct window_result_target : fit_result_target {
    window_result_target() { }
    void bind(filmstack_window *w) { window = w; }
    virtual void notify_change() { window->update_values(); }
    filmstack_window *window;
};

class regress_pro_window : public FXMainWindow {
    FXDECLARE(regress_pro_window)

protected:
    struct spectrum *spectrum;
    fit_recipe *recipe;

    FXString recipeFilename;

protected:
    filmstack_window *main_filmstack_window;
    recipe_window *main_recipe_window;
    filmstack_window *result_filmstack_window;
    disp_fit_window *m_disp_fit_window;

    dataset_window *my_dataset_window;
    batch_window *my_batch_window;
    bool m_enlarged_window;

    FXMenuBar         *menubar;
    FXStatusBar       *statusbar;

    FXMenuPane        *filemenu;
    FXMenuPane        *editmenu;
    FXMenuPane        *spectrmenu;
    FXMenuPane        *dispmenu;
    FXMenuPane        *fitmenu;
    FXMenuPane        *helpmenu;

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

    long onCmdDatasetEdit(FXObject*,FXSelector,void*);
    long onCmdLoadSpectra(FXObject*,FXSelector,void*);
    long onCmdRecipeSaveAs(FXObject*,FXSelector,void*);
    long onCmdRecipeLoad(FXObject*,FXSelector,void*);
    long onCmdDispersOptim(FXObject*,FXSelector,void*);
    long onCmdRunFit(FXObject*,FXSelector,void*);
    long onCmdInteractiveFit(FXObject*,FXSelector,void*);
    long onCmdRunMultiFit(FXObject*,FXSelector,void*);
    long onCmdRunSimul(FXObject*,FXSelector,void*);
    long onCmdBatchWindow(FXObject*,FXSelector,void*);
    long onCmdAbout(FXObject*,FXSelector,void*);
    long onCmdStackChange(FXObject*,FXSelector,void*);
    long onCmdStackShift(FXObject*,FXSelector,void*);
    long onUpdate(FXObject*,FXSelector,void*);
    long onCmdResultStack(FXObject*,FXSelector,void*);
    long onCmdEngineeringScript(FXObject*,FXSelector,void*);

public:
    enum {
        ID_RECIPE_SAVE = FXMainWindow::ID_LAST,
        ID_RECIPE_LOAD,
        ID_LOAD_SPECTRA,
        ID_DISP_OPTIM,
        ID_RUN_FIT,
        ID_INTERACTIVE_FIT,
        ID_RUN_MULTI_FIT,
        ID_RUN_BATCH,
        ID_RUN_SIMUL,
        ID_ABOUT,
        ID_SCRIPT_TEXT,
        ID_DATASET_EDIT,
        ID_STACK_CHANGE,
        ID_STACK_SHIFT,
        ID_RESULT_STACK,
        ID_ENG_SCRIPT,
        ID_LAST
    };

public:
    regress_pro_window(regress_pro *a);
    virtual void create();
    virtual ~regress_pro_window();

    str_ptr load_spectrum(const char *filename);
    str_ptr load_recipe(const char *filename);
    str_ptr run_fit_command();

    bool scriptMode() const { return regressProApp()->scriptMode(); }

private:
    bool check_spectrum(const char *context);
    void set_stack_result(stack_t *s);
    void update_interactive_fit(fit_engine *fit, const lmfit_result& result);
    void set_spectrum(struct spectrum *s);
    FXString run_fit(fit_engine *fit, seeds_list *fseeds, struct spectrum *fspectrum);
    void save_recipe_as(const FXString& filename);

    interactive_fit *m_interactive_fit;
    fit_window *m_fit_window;
    FXDialogBox *m_filmstack_dialog;

    bool m_result_stack_match;
    bool m_title_dirty;

    window_result_target m_interactive_fit_target;
};

extern "C" {

    extern int process_foxgui_events(void *data, float p, const char *msg);

};
