#ifndef FIT_PANEL_H
#define FIT_PANEL_H

#include <fx.h>
#include <agg2/agg_array.h>

#include "fit_manager.h"
#include "plot_canvas.h"

struct fit_result_target {
    virtual void notify_change() = 0;
    virtual ~fit_result_target() { }
};

class regress_pro;

class fit_panel : public FXHorizontalFrame {
    FXDECLARE(fit_panel)

    struct param_info {
        FXTextField *text_field;
        fit_param_t fp;
        bool selected;
        bool is_dirty;
    };

    agg::pod_array<param_info> m_parameters;

    FXFont* m_bold_font;
    FXScrollWindow *scroll_window;
    FXMatrix *param_matrix;

    FXTextField* m_wl_entry;
    plot_canvas* m_canvas;

    fit_manager* m_fit; // Not owned by the class.
    fit_result_target *m_results_target;

    bool verify_spectral_range(const char *txt, double ps[]);
    bool update_spectral_range(const char *txt);

    param_info* get_parameter_pointer(unsigned k)  {
        return m_parameters.data() + k;
    }
    unsigned    get_parameter_index(param_info* p) {
        return p - m_parameters.data();
    }

    void clear();
    void setup();

protected:
    fit_panel() {};
private:
    fit_panel(const fit_panel&);
    fit_panel &operator=(const fit_panel&);

public:
    fit_panel(fit_manager* fit, FXComposite *p, FXuint opts=0, FXint x=0, FXint y=0, FXint w=0, FXint h=0, FXint pl=DEFAULT_SPACING, FXint pr=DEFAULT_SPACING, FXint pt=DEFAULT_SPACING, FXint pb=DEFAULT_SPACING, FXint hs=DEFAULT_SPACING, FXint vs=DEFAULT_SPACING);

    void bind_result_target(fit_result_target *tgt) { m_results_target = tgt; }

    regress_pro *regressProApp() const { return (regress_pro *) getApp(); }

    void refresh();
    void reload();

    long on_cmd_param_select(FXObject*, FXSelector, void*);
    long on_cmd_param_change(FXObject*, FXSelector, void*);
    long on_update_param(FXObject*, FXSelector, void*);
    long on_cmd_plot_autoscale(FXObject*, FXSelector, void*);
    long on_cmd_spectral_range(FXObject*, FXSelector, void*);
    long on_change_spectral_range(FXObject*, FXSelector, void*);
    long on_cmd_run_fit(FXObject*, FXSelector, void*);

    enum {
        ID_PARAM_SELECT = FXHorizontalFrame::ID_LAST,
        ID_PARAM_VALUE,
        ID_SPECTR_RANGE,
        ID_RUN_FIT,
        ID_PLOT_SCALE,
        ID_LAST
    };
};

#endif
