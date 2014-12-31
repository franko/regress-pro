#include "dispers_ui_edit.h"
#include "disp-ho.h"
#include "icons_all.h"

// Map
FXDEFMAP(fx_disp_window) fx_disp_window_map[]= {
    FXMAPFUNC(SEL_CHANGED, fx_disp_window::ID_NAME, fx_disp_window::on_changed_name),
    FXMAPFUNC(SEL_COMMAND, fx_disp_window::ID_DISP_ELEMENT_ADD, fx_disp_window::on_disp_element_add),
    FXMAPFUNCS(SEL_COMMAND, fx_disp_window::ID_DISP_ELEMENT_DELETE, fx_disp_window::ID_DISP_ELEMENT_DELETE_LAST, fx_disp_window::on_disp_element_delete),
    FXMAPFUNCS(SEL_CHANGED, fx_disp_ho_window::ID_PARAM_0, fx_disp_ho_window::ID_PARAM_LAST, fx_disp_ho_window::on_cmd_value),
};

FXIMPLEMENT(fx_disp_window,FXVerticalFrame,fx_disp_window_map,ARRAYNUMBER(fx_disp_window_map));

fx_disp_window::fx_disp_window(disp_t *d, FXComposite* p, FXuint opts)
: FXVerticalFrame(p, opts), disp(d)
{
    delete_icon = new FXGIFIcon(getApp(), delete_gif);
    add_icon = new FXGIFIcon(getApp(), new_gif);
}

void
fx_disp_window::setup_name()
{
    FXHorizontalFrame *namehf = new FXHorizontalFrame(this, LAYOUT_FILL_X);
    new FXLabel(namehf, "Name ");
    FXTextField *tf = new FXTextField(namehf, 24, this, ID_NAME, FRAME_SUNKEN);
    tf->setText(CSTR(disp->name));
}

fx_disp_window::~fx_disp_window()
{
    delete delete_icon;
    delete add_icon;
}

void fx_disp_window::create()
{
    this->setup_dialog();
    FXVerticalFrame::create();
}

long
fx_disp_window::on_cmd_value(FXObject*, FXSelector sel, void *data)
{
    double *pvalue = this->map_parameter(FXSELID(sel) - ID_PARAM_0);
    double new_value = strtod((FXchar*)data, NULL);
    *pvalue = new_value;
    return 1;
}

long
fx_disp_window::on_changed_name(FXObject *, FXSelector, void *text)
{
    str_copy_c(disp->name, (FXchar *) text);
    return 1;
}

long
fx_disp_window::on_disp_element_add(FXObject*, FXSelector, void *)
{
    this->add_dispersion_element();
    return 1;
}

long
fx_disp_window::on_disp_element_delete(FXObject*, FXSelector sel, void *)
{
    int index = FXSELID(sel) - ID_DISP_ELEMENT_DELETE;
    this->delete_dispersion_element(index);
    return 1;
}

static fx_numeric_field *create_textfield(FXComposite *frame, fx_disp_window *target, FXSelector id)
{
    fx_numeric_field *tf = new fx_numeric_field(frame, 10, target, id, FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
    FXString vstr;
    double *pvalue = target->map_parameter(id - fx_disp_window::ID_PARAM_0);
    vstr.format("%g", *pvalue);
    tf->setText(vstr);
    return tf;
}

void fx_disp_ho_window::setup_dialog()
{
    setup_name();

    matrix = new FXMatrix(this, 6, LAYOUT_SIDE_TOP|MATRIX_BY_COLUMNS);
    new FXLabel(matrix, "");
    new FXLabel(matrix, "Nosc");
    new FXLabel(matrix, "En");
    new FXLabel(matrix, "Eg");
    new FXLabel(matrix, "Nu");
    new FXLabel(matrix, "Phi");

    for (int i = 0; i < disp->disp.ho.nb_hos; i++) {
        FXButton *db = new FXButton(matrix, "", delete_icon, this, ID_DISP_ELEMENT_DELETE + i);
        if (disp->disp.ho.nb_hos == 1) { db->disable(); }
        for (int j = 5*i; j < 5*(i+1); j++) {
            create_textfield(matrix, this, ID_PARAM_0 + j);
        }
    }
    new FXButton(this, "", add_icon, this, ID_DISP_ELEMENT_ADD);
}

double *fx_disp_ho_window::map_parameter(int index)
{
    int i = index / 5, j = index % 5;
    struct ho_params *ho = disp->disp.ho.params + i;
    switch (j) {
        case 0: return &ho->nosc; break;
        case 1: return &ho->en;   break;
        case 2: return &ho->eg;   break;
        case 3: return &ho->nu;   break;
        case 4: return &ho->phi;  break;
        default:
        /* */;
    }
    return NULL;
}

void fx_disp_ho_window::add_dispersion_element()
{
    int n = disp->disp.ho.nb_hos;
    disp_add_ho(disp);
    FXButton *db = new FXButton(matrix, "", delete_icon, this, ID_DISP_ELEMENT_DELETE + n);
    db->create();
    for (int j = 5*n; j < 5*(n+1); j++) {
        FXTextField *tf = create_textfield(matrix, this, ID_PARAM_0 + j);
        tf->create();
    }
    matrix->childAtRowCol(1, 0)->enable();
    this->recalc();
}

void fx_disp_ho_window::delete_dispersion_element(int index)
{
    disp_delete_ho(disp, index);
    for (FXWindow *w = this->getFirst(); w; w = this->getFirst()) {
        delete w;
    }
    this->create();
}

void fx_disp_cauchy_window::setup_dialog()
{
    setup_name();

    FXMatrix *matrix = new FXMatrix(this, 2, LAYOUT_SIDE_TOP|MATRIX_BY_COLUMNS);
    new FXLabel(matrix, "N");
    new FXLabel(matrix, "K");

    for (int j = 0; j < 6; j++) {
        create_textfield(matrix, this, ID_PARAM_0 + j);
    }
}

double *fx_disp_cauchy_window::map_parameter(int index)
{
    disp_cauchy *c = &disp->disp.cauchy;
    if (index < 3) {
        return c->n + index;
    } else {
        return c->k + (index - 3);
    }
    return NULL;
}
