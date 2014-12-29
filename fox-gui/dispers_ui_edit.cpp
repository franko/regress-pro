#include "dispers_ui_edit.h"
#include "disp-ho.h"

// Map
FXDEFMAP(fx_disp_ho_window) fx_disp_ho_window_map[]= {
    FXMAPFUNC(SEL_CHANGED, fx_disp_ho_window::ID_NAME, fx_disp_ho_window::on_changed_name),
    FXMAPFUNCS(SEL_CHANGED, fx_disp_ho_window::ID_PARAM_0, fx_disp_ho_window::ID_PARAM_0 + 5*16, fx_disp_ho_window::on_cmd_value),
};

FXIMPLEMENT(fx_disp_ho_window,FXVerticalFrame,fx_disp_ho_window_map,ARRAYNUMBER(fx_disp_ho_window_map));

static fx_numeric_field *create_textfield(FXComposite *frame, FXObject *target, FXSelector id, double value)
{
    fx_numeric_field *tf = new fx_numeric_field(frame, 10, target, id, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
    FXString vstr;
    vstr.format("%g", value);
    tf->setText(vstr);
    return tf;
}


fx_disp_ho_window::fx_disp_ho_window(disp_t *d, FXComposite* p, FXuint opts)
    : FXVerticalFrame(p, opts), m_disp(d)
{
    FXHorizontalFrame *namehf = new FXHorizontalFrame(this, LAYOUT_FILL_X);
    new FXLabel(namehf, "Name");
    FXTextField *tf = new FXTextField(namehf, 24, this, ID_NAME, LAYOUT_FILL_X);
    tf->setText(CSTR(m_disp->name));

    FXMatrix *matrix = new FXMatrix(this, 6, LAYOUT_SIDE_TOP|LAYOUT_FILL_Y|MATRIX_BY_COLUMNS);
    new FXLabel(matrix, "#");
    new FXLabel(matrix, "Nosc");
    new FXLabel(matrix, "En");
    new FXLabel(matrix, "Eg");
    new FXLabel(matrix, "Nu");
    new FXLabel(matrix, "Phi");

    field = new fx_numeric_field*[m_disp->disp.ho.nb_hos * 5];

    char nbstr[32];
    for (int i = 0; i < m_disp->disp.ho.nb_hos; i++) {
        struct ho_params *ho = m_disp->disp.ho.params + i;
        int k = 5*i;
        sprintf(nbstr, "%d", i + 1);
        new FXLabel(matrix, nbstr);
        field[k+0] = create_textfield(matrix, this, ID_PARAM_0 + k+0, ho->nosc);
        field[k+1] = create_textfield(matrix, this, ID_PARAM_0 + k+1, ho->en);
        field[k+2] = create_textfield(matrix, this, ID_PARAM_0 + k+2, ho->eg);
        field[k+3] = create_textfield(matrix, this, ID_PARAM_0 + k+3, ho->nu);
        field[k+4] = create_textfield(matrix, this, ID_PARAM_0 + k+4, ho->phi);
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

// Map
FXDEFMAP(fx_disp_cauchy_window) fx_disp_cauchy_window_map[]= {
    FXMAPFUNC(SEL_CHANGED, fx_disp_cauchy_window::ID_NAME, fx_disp_cauchy_window::on_changed_name),
    FXMAPFUNCS(SEL_CHANGED, fx_disp_cauchy_window::ID_PARAM_0, fx_disp_cauchy_window::ID_PARAM_0 + 5, fx_disp_cauchy_window::on_cmd_value),
};

FXIMPLEMENT(fx_disp_cauchy_window,FXVerticalFrame,fx_disp_cauchy_window_map,ARRAYNUMBER(fx_disp_cauchy_window_map));

fx_disp_cauchy_window::fx_disp_cauchy_window(disp_t *d, FXComposite* p, FXuint opts)
    : FXVerticalFrame(p, opts), m_disp(d)
{
    FXHorizontalFrame *namehf = new FXHorizontalFrame(this, LAYOUT_FILL_X);
    new FXLabel(namehf, "Name");
    FXTextField *tf = new FXTextField(namehf, 24, this, ID_NAME, LAYOUT_FILL_X);
    tf->setText(CSTR(m_disp->name));

    FXMatrix *matrix = new FXMatrix(this, 2, LAYOUT_SIDE_TOP|MATRIX_BY_COLUMNS);
    new FXLabel(matrix, "N");
    new FXLabel(matrix, "K");

    disp_cauchy *cauchy = &m_disp->disp.cauchy;
    n0 = create_textfield(matrix, this, ID_PARAM_0 + 0, cauchy->n[0]);
    n1 = create_textfield(matrix, this, ID_PARAM_0 + 1, cauchy->n[1]);
    n2 = create_textfield(matrix, this, ID_PARAM_0 + 2, cauchy->n[2]);
    k0 = create_textfield(matrix, this, ID_PARAM_0 + 3, cauchy->k[0]);
    k1 = create_textfield(matrix, this, ID_PARAM_0 + 4, cauchy->k[1]);
    k2 = create_textfield(matrix, this, ID_PARAM_0 + 5, cauchy->k[2]);
}

fx_disp_cauchy_window::~fx_disp_cauchy_window()
{ }

long
fx_disp_cauchy_window::on_changed_name(FXObject *, FXSelector, void *__text)
{
    FXchar *text = (FXchar *) __text;
    str_copy_c(m_disp->name, text);
    return 1;
}

long
fx_disp_cauchy_window::on_cmd_value(FXObject*, FXSelector sel, void *data)
{
    FXint index = FXSELID(sel) - ID_PARAM_0;
    FXchar* vstr = (FXchar*)data;
    double val = strtod(vstr, NULL);
    disp_cauchy *c = &m_disp->disp.cauchy;
    if (index < 3) {
        c->n[index] = val;
    } else {
        c->k[index - 3] = val;
    }
    return 1;
}
