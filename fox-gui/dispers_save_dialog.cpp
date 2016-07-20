#include "dispers_save_dialog.h"
#include "regress_pro.h"
#include "writer.h"
#include "mat_table_write.h"
#include "textfield_utils.h"

static const FXchar disp_patterns[] =
    "Dispersion files (*.dsp)"
    "\nAll Files (*)";

static const FXchar tab_patterns[] =
    "Mat Dispersion files (*.mat)"
    "\nAll Files (*)";

// Map
FXDEFMAP(dispers_save_dialog) dispers_save_dialog_map[]= {
    FXMAPFUNC(SEL_COMMAND, dispers_save_dialog::ID_FORMAT_TABULAR, dispers_save_dialog::on_cmd_format_tabular),
    FXMAPFUNC(SEL_COMMAND, dispers_save_dialog::ID_FORMAT_NATIVE, dispers_save_dialog::on_cmd_format_native),
    FXMAPFUNC(SEL_COMMAND, dispers_save_dialog::ID_FILE_SELECT, dispers_save_dialog::on_cmd_file_select),
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
    new FXOption(m_format_pane, "Native", NULL, this, ID_FORMAT_NATIVE, FRAME_RAISED|JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
    new FXOption(m_format_pane, "Tabular", NULL, this, ID_FORMAT_TABULAR, FRAME_RAISED|JUSTIFY_HZ_APART|ICON_AFTER_TEXT);

    m_sampling_pane = new FXPopup(this);
    new FXOption(m_sampling_pane, "Uniform", NULL, this, ID_SAMPLING_UNIFORM, FRAME_RAISED|JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
    FXOption *nat_option = new FXOption(m_sampling_pane, "Native", NULL, this, ID_SAMPLING_NATIVE, FRAME_RAISED|JUSTIFY_HZ_APART|ICON_AFTER_TEXT);

    if (!disp_is_tabular(disp)) {
        nat_option->disable();
    }

    FXHorizontalFrame *bbox = new FXHorizontalFrame(top, LAYOUT_FILL_X);
    FXVerticalFrame *fvf = new FXVerticalFrame(bbox, LAYOUT_FILL_Y);
    new FXLabel(fvf, "Format");
    m_format_menu = new FXOptionMenu(fvf, m_format_pane, FRAME_RAISED|JUSTIFY_HZ_APART|ICON_AFTER_TEXT);

    FXGroupBox *sgroup = new FXGroupBox(bbox, "Sampling", GROUPBOX_NORMAL|FRAME_GROOVE);
    m_sampling_menu = new FXOptionMenu(sgroup, m_sampling_pane, FRAME_RAISED|JUSTIFY_HZ_APART|ICON_AFTER_TEXT|LAYOUT_SIDE_LEFT);

    FXMatrix *matrix = new FXMatrix(sgroup, 3, LAYOUT_SIDE_TOP|MATRIX_BY_COLUMNS|LAYOUT_SIDE_RIGHT);
    new FXLabel(matrix, "Start");
    new FXLabel(matrix, "End");
    new FXLabel(matrix, "Step");
    m_sampling_start = new FXTextField(matrix, 8, this, ID_SAMPLING_START, FRAME_SUNKEN|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
    m_sampling_end = new FXTextField(matrix, 8, this, ID_SAMPLING_END, FRAME_SUNKEN|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
    m_sampling_step = new FXTextField(matrix, 8, this, ID_SAMPLING_STEP, FRAME_SUNKEN|TEXTFIELD_REAL|LAYOUT_FILL_ROW);

    disable_sampling();
}

dispers_save_dialog::~dispers_save_dialog()
{
    delete m_format_pane;
    delete m_sampling_pane;
}

void dispers_save_dialog::disable_sampling()
{
    m_sampling_menu->disable();
}

void dispers_save_dialog::enable_sampling()
{
    m_sampling_menu->enable();
}

long
dispers_save_dialog::on_cmd_format_tabular(FXObject *, FXSelector, void *)
{
    m_filebox->setPatternList(tab_patterns);
    enable_sampling();
    return 1;
}

long
dispers_save_dialog::on_cmd_format_native(FXObject *, FXSelector, void *)
{
    m_filebox->setPatternList(disp_patterns);
    disable_sampling();
    return 1;
}

long
dispers_save_dialog::on_update_sampling(FXObject *sender, FXSelector sel, void *)
{
    FXuint msgid = (use_sampling() ? ID_ENABLE : ID_DISABLE);
    sender->handle(this, FXSEL(SEL_COMMAND, msgid), NULL);
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

int
dispers_save_dialog::save_dispersion()
{
    FXString filename = m_filebox->getFilename();
    regress_pro_app()->disp_dir = m_filebox->getDirectory();
    bool use_native = format_is_native();
    if (filename.find('.') < 0) {
        if (use_native) {
            filename.append(".dsp");
        } else {
            filename.append(".mat");
        }
    }

    if (use_native) {
        writer_t *w = writer_new();
        if (disp_write(w, m_disp)) {
            FXMessageBox::error(getShell(), MBOX_OK, "Save Dispersion", "Error saving dispersion file.");
            writer_free(w);
            return 1;
        }
        if (writer_save_tofile(w, filename.text())) {
            FXMessageBox::error(getShell(), MBOX_OK, "Save Dispersion", "Error writing file %s.", filename.text());
        }
        writer_free(w);
    } else {
        file_writer ostream;
        if (!ostream.open(filename.text())) return 1;
        if (use_sampling() || m_disp->type == DISP_TABLE) {
            double sstart, send, sstep;
            if (use_sampling()) {
                if (get_sampling_values(&sstart, &send, &sstep)) return 1;
            } else {
                sstart = m_disp->disp.table.lambda_min;
                send = m_disp->disp.table.lambda_max;
                sstep = m_disp->disp.table.lambda_stride;
            }
            uniform_sampler usampler(sstart, send, sstep);
            disp_source<uniform_sampler> src(m_disp, &usampler);
            mat_table_write(disp_get_name(m_disp), &ostream, &src);
        } else if (m_disp->type == DISP_SAMPLE_TABLE) {
            dst_source src(&m_disp->disp.sample_table);
            mat_table_write(disp_get_name(m_disp), &ostream, &src);
        }
    }
    return 0;
}

bool dispers_save_dialog::use_sampling() const
{
    return (m_sampling_menu->isEnabled() && m_sampling_menu->getCurrentNo() == 0);
}

long
dispers_save_dialog::on_cmd_file_select(FXObject *, FXSelector, void *)
{
    if (use_sampling()) {
        double a, b, c;
        if (get_sampling_values(&a, &b, &c)) {
            FXMessageBox::error(getShell(), MBOX_OK, "Save Dispersion", "Invalid sampling specification.");
            return 1;
        }
    }
    this->handle(this, FXSEL(SEL_COMMAND, ID_ACCEPT), NULL);
    return 1;
}
