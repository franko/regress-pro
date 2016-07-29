#ifndef DISPERS_SAVE_DIALOG_H
#define DISPERS_SAVE_DIALOG_H

#include <fx.h>

#include "dispers.h"

class regress_pro;

class dispers_save_dialog : public FXDialogBox {
    FXDECLARE(dispers_save_dialog)
protected:
    dispers_save_dialog() {};
private:
    dispers_save_dialog(const dispers_save_dialog&);
    dispers_save_dialog &operator=(const dispers_save_dialog&);
public:
    dispers_save_dialog(const disp_t *d, FXWindow* owner, const FXString& name, FXuint opts=0, FXint x=0, FXint y=0, FXint w=540, FXint h=360);
    ~dispers_save_dialog();

    regress_pro *regress_pro_app() { return (regress_pro *) getApp(); }

    int save_dispersion();

    long on_cmd_format_tabular(FXObject *, FXSelector, void *);
    long on_cmd_format_native(FXObject *, FXSelector, void *);
    long on_cmd_sampling_option(FXObject *, FXSelector, void *);
    long on_upd_sampling_option(FXObject *, FXSelector, void *);
    long on_cmd_sampling_radio(FXObject *, FXSelector, void *);
    long on_update_sampling(FXObject *, FXSelector, void *);
    long on_update_sampling_native(FXObject *, FXSelector, void *);
    long on_update_sampling_radio(FXObject *, FXSelector, void *);
    long on_update_sampling_tolerance(FXObject *, FXSelector, void *);
    long on_cmd_file_select(FXObject *, FXSelector, void *);

    bool format_is_native() const { return m_format_menu->getCurrentNo() == 0; }

    enum {
        SAMPLING_NATIVE = 0,
        SAMPLING_OPTIM  = 1,
        SAMPLING_NONE   = 0 << 1,
        SAMPLING_AUTO   = 1 << 1,
        SAMPLING_UNIF   = 2 << 1,

        SAMPLING_OPTIM_MASK = 1,
        SAMPLING_TYPE_MASK  = 3 << 1,
    };

    enum {
        ID_FORMAT_NATIVE = FXDialogBox::ID_LAST,
        ID_FORMAT_TABULAR,
        ID_FILE_SELECT,
        ID_SAMPLING_OPTION,
        ID_SAMPLING_AUTO,
        ID_SAMPLING_UNIFORM,
        ID_SAMPLING_OPTIMIZED,
        ID_SAMPLING_TOL,
        ID_SAMPLING_START,
        ID_SAMPLING_END,
        ID_SAMPLING_STEP,
        ID_LAST
    };
private:
    int get_sampling_values(double *sstart, double *send, double *sstep) const;
    bool use_uniform_sampling() const { return ((m_sampling_type & SAMPLING_TYPE_MASK) == SAMPLING_UNIF); }

    const disp_t *m_disp;
    FXFileSelector *m_filebox;
    FXPopup *m_format_pane, *m_sampling_pane;
    FXOptionMenu *m_format_menu;
    FXListBox *m_sampling_listbox;
    FXTextField *m_sampling_start, *m_sampling_end, *m_sampling_step;
    FXTextField *m_sampling_tol_textfield;
    unsigned m_sampling_type;
};

#endif
