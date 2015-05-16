#ifndef DISPERS_UI_EDIT_H
#define DISPERS_UI_EDIT_H

#include <fx.h>

#include "dispers.h"
#include "fx_numeric_field.h"

class regress_pro;

class fx_disp_window : public FXVerticalFrame {
    FXDECLARE(fx_disp_window)

protected:
    fx_disp_window() {};
private:
    fx_disp_window(const fx_disp_window&);
    fx_disp_window &operator=(const fx_disp_window&);

public:
    fx_disp_window(disp_t *d, FXComposite *p,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);

    virtual void setup_dialog() {}
    virtual void add_dispersion_element() {}
    virtual void delete_dispersion_element(int index) {}
    virtual double *map_parameter(int index) { return 0; }

    void setup_name();
    void setup();
    void reload();

    regress_pro *regressProApp() { return (regress_pro *) getApp(); }

    long on_cmd_value(FXObject*, FXSelector, void *);
    long on_changed_name(FXObject*, FXSelector, void *);
    long on_disp_element_add(FXObject*, FXSelector, void *);
    long on_disp_element_delete(FXObject*, FXSelector, void *);

    enum {
        ID_NAME = FXVerticalFrame::ID_LAST,
        ID_PARAM_0,
        ID_PARAM_LAST = ID_PARAM_0 + 5 * 16,
        ID_DISP_ELEMENT_DELETE,
        ID_DISP_ELEMENT_DELETE_LAST = ID_DISP_ELEMENT_DELETE + 16,
        ID_DISP_ELEMENT_ADD,
        ID_LAST
    };

protected:
    // This is just a reference. The class is not owner of this object.
    disp_t *disp;
};

class fx_disp_ho_window : public fx_disp_window {
public:
    fx_disp_ho_window(disp_t *d, FXComposite *p, FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING)
    : fx_disp_window(d, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
    { }

    virtual void setup_dialog();
    virtual void add_dispersion_element();
    virtual void delete_dispersion_element(int index);
    virtual double *map_parameter(int index);
private:
    FXMatrix *matrix;
    FXVerticalFrame *vframe;
};

class fx_disp_cauchy_window : public fx_disp_window {
public:
    fx_disp_cauchy_window(disp_t *d, FXComposite *p, FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING)
    : fx_disp_window(d, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
    { }

    virtual void setup_dialog();
    virtual double *map_parameter(int index);
};

class fx_disp_lookup_window : public fx_disp_window {
    FXDECLARE(fx_disp_lookup_window)

protected:
    fx_disp_lookup_window() {};
private:
    fx_disp_lookup_window(const fx_disp_window&);
    fx_disp_lookup_window &operator=(const fx_disp_window&);

public:
    fx_disp_lookup_window(disp_t *d, FXComposite *p, FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
    virtual ~fx_disp_lookup_window();

    virtual void create();

    virtual void setup_dialog();
    virtual void add_dispersion_element();
    virtual double *map_parameter(int index);

    long on_changed_component_name(FXObject *, FXSelector, void *);
    long on_button_menu_component(FXObject *, FXSelector, void *);
    long on_cmd_delete_comp(FXObject *, FXSelector, void *);
    long on_cmd_insert_comp(FXObject *, FXSelector, void *);
    long on_cmd_edit_comp(FXObject *, FXSelector, void *);
    long on_cmd_replace_comp(FXObject *, FXSelector, void *);

    enum {
        ID_COMPONENT_NAME = fx_disp_window::ID_LAST,
        ID_COMPONENT_NAME_LAST = ID_COMPONENT_NAME + 64,
        ID_MENU_COMPONENT,
        ID_MENU_COMPONENT_LAST = ID_MENU_COMPONENT + 64,
        ID_DELETE_COMP,
        ID_REPLACE_COMP,
        ID_INSERT_COMP,
        ID_EDIT_COMP,
        ID_LAST
    };

private:
    void add_matrix_component(int index, bool create = false);

    FXFont *small_font;
    FXMatrix *matrix;
    FXVerticalFrame *vframe;
    FXMenuPane *popupmenu;
    int selected_component;
};

struct table_nk_iterator {
    table_nk_iterator() {}
    virtual ~table_nk_iterator() {}
    virtual int rows() const = 0;
    virtual float get_nk_row(int index, float *n, float *k) = 0;
};

struct disp_table_rows_iter : table_nk_iterator {
    disp_table_rows_iter(disp_table *dt): m_table(dt) {}
    virtual int rows() const { return m_table->points_number; }
    virtual float get_nk_row(int index, float *n, float *k) {
        *n = data_table_get(m_table->table_ref, index, 0);
        *k = data_table_get(m_table->table_ref, index, 1);
        return m_table->lambda_min + m_table->lambda_stride * index;
    }
private:
    disp_table *m_table;
};

struct disp_sample_table_rows_iter : table_nk_iterator {
    disp_sample_table_rows_iter(disp_sample_table *dt): m_table(dt) {}
    virtual int rows() const { return m_table->len; }
    virtual float get_nk_row(int index, float *n, float *k) {
        double lw, ln, lk;
        disp_sample_table_get_sample(m_table, index, &lw, &ln, &lk);
        *n = float(ln);
        *k = float(lk);
        return float(lw);
    }
private:
    disp_sample_table *m_table;
};

static table_nk_iterator *new_table_iterator(disp_t *d) {
    if (d->type == DISP_TABLE) {
        return new disp_table_rows_iter(&d->disp.table);
    } else if (d->type == DISP_SAMPLE_TABLE) {
        return new disp_sample_table_rows_iter(&d->disp.sample_table);
    }
    return NULL;
}

class fx_disp_table_window : public fx_disp_window {
public:
    fx_disp_table_window(disp_t *d, FXComposite *p, FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING):
        fx_disp_window(d, p, opts, x, y, w, h, pl, pr, pt, pb, hs, vs),
        m_iterator(new_table_iterator(d))
    {}
    virtual ~fx_disp_table_window() {
        delete m_iterator;
    }

    virtual void setup_dialog();
private:
    table_nk_iterator *m_iterator;
    FXTable *m_table;
};

// Create a new fx_disp_window (or a derived class instance) that corresponds to the
// given dispersion.
extern fx_disp_window *new_disp_window(disp_t *d, FXComposite *comp);

#endif
