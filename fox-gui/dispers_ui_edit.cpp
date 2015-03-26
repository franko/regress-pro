#include "dispers_ui_edit.h"
#include "disp-ho.h"
#include "icons_all.h"
#include "dispers_chooser.h"

// Map
FXDEFMAP(fx_disp_window) fx_disp_window_map[]= {
    FXMAPFUNC(SEL_CHANGED, fx_disp_window::ID_NAME, fx_disp_window::on_changed_name),
    FXMAPFUNC(SEL_COMMAND, fx_disp_window::ID_DISP_ELEMENT_ADD, fx_disp_window::on_disp_element_add),
    FXMAPFUNCS(SEL_COMMAND, fx_disp_window::ID_DISP_ELEMENT_DELETE, fx_disp_window::ID_DISP_ELEMENT_DELETE_LAST, fx_disp_window::on_disp_element_delete),
    FXMAPFUNCS(SEL_CHANGED, fx_disp_ho_window::ID_PARAM_0, fx_disp_ho_window::ID_PARAM_LAST, fx_disp_ho_window::on_cmd_value),
};

FXIMPLEMENT(fx_disp_window,FXVerticalFrame,fx_disp_window_map,ARRAYNUMBER(fx_disp_window_map));

fx_disp_window::fx_disp_window(disp_t *d, FXComposite* p, FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs)
: FXVerticalFrame(p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs), disp(d)
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
    fx_numeric_field *tf = new fx_numeric_field(frame, 8, target, id, FRAME_SUNKEN|TEXTFIELD_REAL|LAYOUT_FILL_ROW);
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

    for (int p = 0; p < 6; p++) {
        int i = p / 2, j = p % 2;
        create_textfield(matrix, this, ID_PARAM_0 + i + 3 * j);
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

fx_disp_window *new_disp_window(disp_t *d, FXComposite *comp)
{
    const FXint opts = LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_GROOVE;
    fx_disp_window *dispwin;
    if (d->type == DISP_HO) {
        dispwin = new fx_disp_ho_window(d, comp, opts);
    } else if (d->type == DISP_CAUCHY) {
        dispwin = new fx_disp_cauchy_window(d, comp, opts);
    } else if (d->type == DISP_LOOKUP) {
        dispwin = new fx_disp_lookup_window(d, comp, opts);
    } else {
        dispwin = new fx_disp_window(d, comp, opts);
    }
    return dispwin;
}

// Map
FXDEFMAP(fx_disp_lookup_window) fx_disp_lookup_window_map[]= {
    FXMAPFUNC(SEL_COMMAND, fx_disp_lookup_window::ID_COMPONENT_NAME, fx_disp_lookup_window::on_cmd_component_name),
};

FXIMPLEMENT(fx_disp_lookup_window,fx_disp_window,fx_disp_lookup_window_map,ARRAYNUMBER(fx_disp_lookup_window_map));

void fx_disp_lookup_window::setup_dialog()
{
    setup_name();

    matrix = new FXMatrix(this, 3, LAYOUT_SIDE_TOP|MATRIX_BY_COLUMNS);
    new FXLabel(matrix, "");
    new FXLabel(matrix, "Dispersion");
    new FXLabel(matrix, "P");

    disp_lookup *lookup = &disp->disp.lookup;
    for (int i = 0; i < lookup->nb_comps; i++) {
        FXButton *db = new FXButton(matrix, "", delete_icon, this, ID_DISP_ELEMENT_DELETE + i);
        if (lookup->nb_comps == 1) { db->disable(); }
        const char *cname = CSTR(lookup->component[i].disp->name);
        FXTextField *tf = new FXTextField(matrix, 32, this, ID_COMPONENT_NAME, FRAME_SUNKEN);
        tf->setText(cname);
        create_textfield(matrix, this, ID_PARAM_0 + i);
    }
    new FXButton(this, "", add_icon, this, ID_DISP_ELEMENT_ADD);
}

double *fx_disp_lookup_window::map_parameter(int index)
{
    return &disp->disp.lookup.component[index].p;
}

void fx_disp_lookup_window::add_dispersion_element()
{
    dispers_chooser chooser(this->getApp());
    if (chooser.execute() != TRUE) return;
    disp_t *comp = chooser.get_dispersion();
    disp_lookup *lookup = &disp->disp.lookup;
    int n = lookup->nb_comps;
    double p1 = lookup->component[0].p, p2 = lookup->component[n-1].p;
    double p0 = p2 + (n > 1 ? (p2 - p1) / (n - 1) : 1.0);
    disp_lookup_add_comp(disp, comp, p0);
    FXButton *db = new FXButton(matrix, "", delete_icon, this, ID_DISP_ELEMENT_DELETE + n);
    db->create();
    FXTextField *tf1 = new FXTextField(matrix, 32, this, ID_COMPONENT_NAME, FRAME_SUNKEN);
    tf1->setText(CSTR(lookup->component[n].disp->name));
    FXTextField *tf2 = create_textfield(matrix, this, ID_PARAM_0 + n);
    tf1->create();
    tf2->create();
    matrix->childAtRowCol(1, 0)->enable();
    this->recalc();
}

void fx_disp_lookup_window::delete_dispersion_element(int index)
{
}

long fx_disp_lookup_window::on_cmd_component_name(FXObject *, FXSelector, void *)
{
    return 0;
}