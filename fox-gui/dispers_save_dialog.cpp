#include "dispers_save_dialog.h"
#include "regress_pro.h"
#include "writer.h"
#include "mat_table_write.h"
#include "textfield_utils.h"
#include "dispers_sampling_optim.h"

static const FXchar disp_patterns[] =
    "Dispersion files (*.dsp)"
    "\nAll Files (*)";

static const FXchar tab_patterns[] =
    "Text format (*.mat,*.txt,*.nkf)"
    "\nAll Files (*)";

// Map
FXDEFMAP(dispers_save_dialog) dispers_save_dialog_map[]= {
    FXMAPFUNC(SEL_COMMAND, dispers_save_dialog::ID_FORMAT_TABULAR, dispers_save_dialog::on_cmd_format_tabular),
    FXMAPFUNC(SEL_COMMAND, dispers_save_dialog::ID_FORMAT_NATIVE, dispers_save_dialog::on_cmd_format_native),
    FXMAPFUNC(SEL_COMMAND, dispers_save_dialog::ID_FILE_SELECT, dispers_save_dialog::on_cmd_file_select),
    FXMAPFUNC(SEL_COMMAND, dispers_save_dialog::ID_SAMPLING_OPTION, dispers_save_dialog::on_cmd_sampling_option),
    FXMAPFUNC(SEL_UPDATE,  dispers_save_dialog::ID_SAMPLING_OPTION, dispers_save_dialog::on_upd_sampling_option),
    FXMAPFUNC(SEL_COMMAND, dispers_save_dialog::ID_SAMPLING_AUTO, dispers_save_dialog::on_cmd_sampling_radio),
    FXMAPFUNC(SEL_COMMAND, dispers_save_dialog::ID_SAMPLING_UNIFORM, dispers_save_dialog::on_cmd_sampling_radio),
    FXMAPFUNC(SEL_UPDATE,  dispers_save_dialog::ID_SAMPLING_AUTO, dispers_save_dialog::on_update_sampling_radio),
    FXMAPFUNC(SEL_UPDATE,  dispers_save_dialog::ID_SAMPLING_UNIFORM, dispers_save_dialog::on_update_sampling_radio),
    FXMAPFUNC(SEL_UPDATE,  dispers_save_dialog::ID_SAMPLING_TOL, dispers_save_dialog::on_update_sampling_tolerance),
    FXMAPFUNCS(SEL_UPDATE, dispers_save_dialog::ID_SAMPLING_START, dispers_save_dialog::ID_SAMPLING_STEP, dispers_save_dialog::on_update_sampling),
};

FXIMPLEMENT(dispers_save_dialog,FXDialogBox,dispers_save_dialog_map,ARRAYNUMBER(dispers_save_dialog_map));

dispers_save_dialog::dispers_save_dialog(const disp_t *disp, FXWindow *owner, const FXString& name, FXuint opts, FXint x, FXint y, FXint w, FXint h):
    FXDialogBox(owner, name, opts|DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE|DECOR_CLOSE,x,y,w,h,0,0,0,0,4,4),
    m_disp(disp)
{
    FXVerticalFrame *top = new FXVerticalFrame(this, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_filebox = new FXFileSelector(top, this, ID_FILE_SELECT, LAYOUT_FILL_X|LAYOUT_FILL_Y);
    m_filebox->acceptButton()->setTarget(this);
    m_filebox->acceptButton()->setSelector(ID_FILE_SELECT);
    m_filebox->cancelButton()->setTarget(this);
    m_filebox->cancelButton()->setSelector(FXDialogBox::ID_CANCEL);

    m_filebox->setPatternList(disp_patterns);
    m_filebox->setDirectory(regress_pro_app()->disp_dir);

    m_format_pane = new FXPopup(this);
    new FXOption(m_format_pane, "Native", nullptr, this, ID_FORMAT_NATIVE, FRAME_RAISED|JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
    new FXOption(m_format_pane, "Tabular", nullptr, this, ID_FORMAT_TABULAR, FRAME_RAISED|JUSTIFY_HZ_APART|ICON_AFTER_TEXT);

    FXHorizontalFrame *bbox = new FXHorizontalFrame(top, LAYOUT_FILL_X);
    FXVerticalFrame *fvf = new FXVerticalFrame(bbox, LAYOUT_FILL_Y);
    new FXLabel(fvf, "Format");
    m_format_menu = new FXOptionMenu(fvf, m_format_pane, FRAME_RAISED|JUSTIFY_HZ_APART|ICON_AFTER_TEXT);

    auto sgroup = new FXGroupBox(bbox, "Wavelength sampling", GROUPBOX_NORMAL|FRAME_GROOVE);
    auto wsampvbox = new FXVerticalFrame(sgroup, LAYOUT_FILL_X);

    auto autohbox = new FXHorizontalFrame(wsampvbox, LAYOUT_FILL_X);
    new FXRadioButton(autohbox, "Automatic", this, ID_SAMPLING_AUTO);
    m_sampling_listbox = new FXListBox(autohbox, this, ID_SAMPLING_OPTION, FRAME_SUNKEN|LISTBOX_NORMAL);
    m_sampling_listbox->appendItem("Native", nullptr, (void *) 0);
    m_sampling_listbox->appendItem("Optimize", nullptr, (void *) 1);
    m_sampling_listbox->setNumVisible(2);
    new FXLabel(autohbox, "tolerance");
    m_sampling_tol_textfield = new FXTextField(autohbox, 8, this, ID_SAMPLING_TOL, FRAME_SUNKEN|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
    m_sampling_tol_textfield->setText("0.001");

    auto unifhbox = new FXHorizontalFrame(wsampvbox, LAYOUT_FILL_X);
    new FXRadioButton(unifhbox, "", this, ID_SAMPLING_UNIFORM);
    new FXLabel(unifhbox, "from");
    m_sampling_start = new FXTextField(unifhbox, 8, this, ID_SAMPLING_START, FRAME_SUNKEN|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
    new FXLabel(unifhbox, "to");
    m_sampling_end = new FXTextField(unifhbox, 8, this, ID_SAMPLING_END, FRAME_SUNKEN|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
    new FXLabel(unifhbox, "step");
    m_sampling_step = new FXTextField(unifhbox, 8, this, ID_SAMPLING_STEP, FRAME_SUNKEN|TEXTFIELD_REAL|LAYOUT_FILL_ROW);

    m_sampling_type = SAMPLING_NONE;
}

dispers_save_dialog::~dispers_save_dialog()
{
    delete m_format_pane;
}

long
dispers_save_dialog::on_cmd_format_tabular(FXObject *, FXSelector, void *)
{
    m_filebox->setPatternList(tab_patterns);
    if (disp_is_tabular(m_disp)) {
        m_sampling_type = SAMPLING_AUTO | SAMPLING_NATIVE;
    } else {
        m_sampling_type = SAMPLING_AUTO | SAMPLING_OPTIM;
    }
    return 1;
}

long
dispers_save_dialog::on_cmd_format_native(FXObject *, FXSelector, void *)
{
    m_filebox->setPatternList(disp_patterns);
    m_sampling_type = SAMPLING_NONE;
    return 1;
}

long
dispers_save_dialog::on_cmd_sampling_radio(FXObject *, FXSelector sel, void *)
{
    unsigned bit = FXSELID(sel) == ID_SAMPLING_AUTO ? SAMPLING_AUTO : SAMPLING_UNIF;
    m_sampling_type = (m_sampling_type & SAMPLING_OPTIM_MASK) | bit;
    return 1;
}

long
dispers_save_dialog::on_update_sampling(FXObject *sender, FXSelector sel, void *)
{
    FXuint msgid = (use_uniform_sampling() ? ID_ENABLE : ID_DISABLE);
    sender->handle(this, FXSEL(SEL_COMMAND, msgid), nullptr);
    return 1;
}

long
dispers_save_dialog::on_cmd_sampling_option(FXObject *sender, FXSelector sel, void *index)
{
    unsigned bit = (index == 0 ? SAMPLING_NATIVE : SAMPLING_OPTIM);
    m_sampling_type = (m_sampling_type & SAMPLING_TYPE_MASK) | bit;
    return 1;
}

long
dispers_save_dialog::on_upd_sampling_option(FXObject *sender, FXSelector sel, void *)
{
    unsigned bit = (m_sampling_type & SAMPLING_OPTIM_MASK);
    if (bit == SAMPLING_NATIVE) {
        m_sampling_listbox->setCurrentItem(0);
    } else {
        m_sampling_listbox->setCurrentItem(1);
    }
    FXuint msgid = ((m_sampling_type & SAMPLING_TYPE_MASK) != SAMPLING_AUTO || !disp_is_tabular(m_disp) ? ID_DISABLE : ID_ENABLE);
    m_sampling_listbox->handle(this, FXSEL(SEL_COMMAND, msgid), nullptr);
    return 1;
}

long
dispers_save_dialog::on_update_sampling_radio(FXObject *sender, FXSelector sel, void *)
{
    FXuint msgid = (FXSELID(sel) == ID_SAMPLING_AUTO ?
        (use_uniform_sampling() ? ID_UNCHECK : ID_CHECK) :
        (use_uniform_sampling() ? ID_CHECK   : ID_UNCHECK));
    sender->handle(this, FXSEL(SEL_COMMAND, msgid), nullptr);

    FXuint enable_msgid = (m_sampling_type & SAMPLING_TYPE_MASK) == SAMPLING_NONE ? ID_DISABLE : ID_ENABLE;
    sender->handle(this, FXSEL(SEL_COMMAND, enable_msgid), nullptr);
    return 1;
}

long
dispers_save_dialog::on_update_sampling_tolerance(FXObject *sender, FXSelector sel, void *)
{
    FXuint msgid = (m_sampling_type == (SAMPLING_AUTO | SAMPLING_OPTIM)) ? ID_ENABLE : ID_DISABLE;
    sender->handle(this, FXSEL(SEL_COMMAND, msgid), nullptr);
    return 1;
}

int
dispers_save_dialog::get_sampling_values(double *sstart, double *send, double *sstep) const
{
    if (textfield_get_double(m_sampling_start, sstart)) return 1;
    if (textfield_get_double(m_sampling_end, send)) return 1;
    if (textfield_get_double(m_sampling_step, sstep)) return 1;
    if (*send <= *sstart || *sstep <= 0.0 || (*send - *sstart)/(*send) > 1.0e4) return 1;
    return 0;
}

static int write_native_format(const disp_t *disp, const char *filename) {
    writer_t *w = writer_new();
    disp_write(w, disp);
    if (writer_save_tofile(w, filename)) {
        return 1;
    }
    writer_free(w);
    return 0;
}

int
dispers_save_dialog::save_dispersion()
{
    FXString filename = m_filebox->getFilename();
    regress_pro_app()->disp_dir = m_filebox->getDirectory();
    bool use_native = format_is_native();
    int tabular_format = TABULAR_FORMAT_NONE;
    if (filename.find('.') < 0) {
        if (use_native) {
            filename.append(".dsp");
        } else {
            filename.append(".mat");
            tabular_format = TABULAR_FORMAT_MAT;
        }
    } else {
        FXString ext = filename.rafter('.');
        if (ext == "txt") {
            tabular_format = TABULAR_FORMAT_TXT;
        } else if (ext == "nkf") {
            tabular_format = TABULAR_FORMAT_NKF;
        } else if (ext == "mat") {
            tabular_format = TABULAR_FORMAT_MAT;
        }
        if (use_native && tabular_format != TABULAR_FORMAT_NONE) {
            FXMessageBox::error(getShell(), MBOX_OK, "Save Dispersion", "Please choose \".dsp\" for native format.");
            return 1;
        }
        if (!use_native && ext == "dsp") {
            if (m_disp->type == DISP_SAMPLE_TABLE) {
                use_native = true;
            } else {
                const double tol = 1e-3;
                auto wavelengths = optimize_sampling_points_c(m_disp, tol);
                disp_t *disp_optim = dispersion_from_sampling_points(m_disp, wavelengths, disp_get_name(m_disp));
                if (write_native_format(disp_optim, filename.text())) {
                    FXMessageBox::error(getShell(), MBOX_OK, "Save Dispersion", "Error writing file %s.", filename.text());
                }
                disp_free(disp_optim);
                return 0;
            }
        }
    }

    if (use_native) {
        if (write_native_format(m_disp, filename.text())) {
            FXMessageBox::error(getShell(), MBOX_OK, "Save Dispersion", "Error writing file %s.", filename.text());
        }
    } else {
        file_writer ostream;
        if (!ostream.open(filename.text())) return 1;

        if (use_uniform_sampling()) {
            double sstart, send, sstep;
            if (get_sampling_values(&sstart, &send, &sstep)) return 1;
            uniform_sampler usampler(sstart, send, sstep);
            disp_source<uniform_sampler> src(m_disp, &usampler);
            mat_table_write(disp_get_name(m_disp), &ostream, &src, tabular_format);
        } else {
            if ((m_sampling_type & SAMPLING_OPTIM_MASK) == SAMPLING_NATIVE) {
                sampled_dispersion_source src(m_disp);
                mat_table_write(disp_get_name(m_disp), &ostream, &src, tabular_format);
            } else {
                double tol;
                textfield_get_double(m_sampling_tol_textfield, &tol);
                disp_t *disp_optim;
                if (m_disp->type == DISP_SAMPLE_TABLE) {
                    const disp_sample_table *disp_st = &m_disp->disp.sample_table;
                    auto sampling_points = optimize_sampling_points(disp_st, tol);
                    disp_optim = dispersion_from_sampling_points(disp_st, sampling_points, "");
                } else {
                    auto wavelengths = optimize_sampling_points_c(m_disp, tol);
                    disp_optim = dispersion_from_sampling_points(m_disp, wavelengths, "");
                }
                sampled_dispersion_source src(disp_optim);
                mat_table_write(disp_get_name(m_disp), &ostream, &src, tabular_format);
                disp_free(disp_optim);
            }
        }
    }
    return 0;
}

long
dispers_save_dialog::on_cmd_file_select(FXObject *, FXSelector, void *)
{
    if (use_uniform_sampling()) {
        double a, b, c;
        if (get_sampling_values(&a, &b, &c)) {
            FXMessageBox::error(getShell(), MBOX_OK, "Save Dispersion", "Invalid sampling specification.");
            return 1;
        }
    }
    this->handle(this, FXSEL(SEL_COMMAND, ID_ACCEPT), nullptr);
    return 1;
}
