#include "dispers_ui_edit.h"
#include "disp-ho.h"

// Map
FXDEFMAP(fx_disp_ho_window) fx_disp_ho_window_map[]= {
    FXMAPFUNC(SEL_CHANGED, fx_disp_ho_window::ID_NAME, fx_disp_ho_window::on_changed_name),
    FXMAPFUNCS(SEL_CHANGED, fx_disp_ho_window::ID_PARAM_0, fx_disp_ho_window::ID_PARAM_0 + 5*16, fx_disp_ho_window::on_cmd_value),
};

FXIMPLEMENT(fx_disp_ho_window,FXVerticalFrame,fx_disp_ho_window_map,ARRAYNUMBER(fx_disp_ho_window_map));

fx_disp_ho_window::fx_disp_ho_window(disp_t *d, FXComposite* p, FXuint opts)
    : FXVerticalFrame(p, opts), m_disp(d)
{
    FXHorizontalFrame *namehf = new FXHorizontalFrame(this, LAYOUT_FILL_X);
    new FXLabel(namehf, "Name");
    new FXTextField(namehf, 24, this, ID_NAME, LAYOUT_FILL_X);

    FXMatrix *matrix = new FXMatrix(this, 6, LAYOUT_SIDE_TOP|LAYOUT_FILL_Y|MATRIX_BY_COLUMNS);
    new FXLabel(matrix, "#");
    new FXLabel(matrix, "Nosc");
    new FXLabel(matrix, "En");
    new FXLabel(matrix, "Eg");
    new FXLabel(matrix, "Nu");
    new FXLabel(matrix, "Phi");

    field = new fx_numeric_field*[m_disp->disp.ho.nb_hos * 5];

    char nbstr[32];
    FXString vstr;
    for (int i = 0; i < m_disp->disp.ho.nb_hos; i++) {
        struct ho_params *ho = m_disp->disp.ho.params + i;
        int k = 5*i;
        sprintf(nbstr, "%d", i + 1);
        new FXLabel(matrix, nbstr);
        field[k] = new fx_numeric_field(matrix, 10, this, ID_PARAM_0 + 5*i + 0, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
        vstr.format("%g", ho->nosc);
        field[k++]->setText(vstr);
        field[k] = new fx_numeric_field(matrix, 10, this, ID_PARAM_0 + 5*i + 1, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
        vstr.format("%g", ho->en);
        field[k++]->setText(vstr);
        field[k] = new fx_numeric_field(matrix, 10, this, ID_PARAM_0 + 5*i + 2, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
        vstr.format("%g", ho->eg);
        field[k++]->setText(vstr);
        field[k] = new fx_numeric_field(matrix, 10, this, ID_PARAM_0 + 5*i + 3, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
        vstr.format("%g", ho->nu);
        field[k++]->setText(vstr);
        field[k] = new fx_numeric_field(matrix, 10, this, ID_PARAM_0 + 5*i + 4, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
        vstr.format("%g", ho->phi);
        field[k++]->setText(vstr);
    }
}

fx_disp_ho_window::~fx_disp_ho_window()
{
    delete [] field;
}

long
fx_disp_ho_window::on_cmd_value(FXObject*, FXSelector sel, void *data)
{
    FXint index = FXSELID(sel) - ID_PARAM_0;
    int i = index / 5, j = index % 5;
    FXchar* vstr = (FXchar*)data;
    double val = strtod(vstr, NULL);
    struct ho_params *ho = m_disp->disp.ho.params + i;
    fprintf(stderr, ">> %d, %d, value %g\n", i, j, val);
    switch (j) {
        case 0: ho->nosc = val; break;
        case 1: ho->en   = val; break;
        case 2: ho->eg   = val; break;
        case 3: ho->nu   = val; break;
        case 4: ho->phi  = val; break;
        default:
        /* */;
    }
    return 1;
}

long
fx_disp_ho_window::on_changed_name(FXObject *, FXSelector, void *__text)
{
    FXchar *text = (FXchar *) __text;
    str_copy_c(m_disp->name, text);
    return 1;
}
