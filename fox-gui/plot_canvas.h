#ifndef AGG_PLOT_CANVAS_H
#define AGG_PLOT_CANVAS_H

#include <fx.h>

#include <agg2/agg_rendering_buffer.h>

#include "plot_auto.h"
#include "agg_plot_array.h"
#include "vs_object.h"
#include "canvas.h"
#include "plot_clipboard.h"

class plot_canvas : public FXCanvas {
    FXDECLARE(plot_canvas)

public:
    typedef newplot::plot_auto<vs_object, owner_management> plot_type;

    plot_canvas(FXComposite* p,
                FXObject* tgt=NULL, FXSelector sel=0,
                FXuint opts=FRAME_NORMAL,
                FXint x=0, FXint y=0, FXint w=0, FXint h=0) :
        FXCanvas(p, tgt, sel, opts, x, y, w, h),
        m_clipboard_content(0), m_canvas(0), m_img(0), m_dirty_flag(true)
    {}

    virtual ~plot_canvas() {
        delete m_img;
        delete m_canvas;
        delete m_clipboard_content;
    }

    virtual void create();

    plot_type* plot(unsigned i) {
        return m_plots.plot(i);
    }

    unsigned plot_number() const { return m_plots.size(); }

    void add(plot_type* plot) {
        m_plots.add(plot);
        m_dirty_flag = true;
    }

    void update_limits() {
        for(unsigned i = 0; i < m_plots.size(); i++) {
            m_plots.plot(i)->update_limits();
        }
        m_dirty_flag = true;
    }

    void clear_plots() {
        m_plots.clear();
        m_dirty_flag = true;
    }

    void set_dirty(bool flag) {
        m_dirty_flag = true;
    }
    bool dirty() const {
        return m_dirty_flag;
    }

    long on_cmd_paint(FXObject *, FXSelector, void *);
    long on_update(FXObject *, FXSelector, void *);
    long on_clipboard_gained(FXObject *, FXSelector, void *);
    long on_clipboard_lost(FXObject *, FXSelector, void *);
    long on_clipboard_request(FXObject *, FXSelector, void *);

    void get_data(int plot_index, line_data *ld)
    {
        plot(plot_index)->xy_tables(&ld->tables);
        ld->names.add(new str("Measured"));
        if (ld->tables.size() > 1) {
            ld->names.add(new str("Theory"));
        }
    }

    void acquire_clipboard();

protected:
    plot_canvas() {};
private:
    plot_canvas(const plot_canvas&);
    plot_canvas &operator=(const plot_canvas&);

protected:
    static FXDragType urilist_type;
    static FXDragType html_type;
    static FXDragType csv_type;
    static FXDragType utf8_type;
    static FXDragType utf16_type;

private:
    void draw_plot(FXEvent*);
    void prepare_image_buffer(int ww, int hh);
    void ensure_canvas_size(int ww, int hh);

    newplot::plot_array<plot_type, newplot::vertical_layout> m_plots;

    agg::rendering_buffer m_rbuf;
    vector_objects<plot_content> *m_clipboard_content;

    canvas* m_canvas;
    FXImage* m_img;

    bool m_dirty_flag;
};

extern void add_new_simple_plot(plot_canvas* canvas, vs_object* v, const char *title);

extern void add_new_plot(plot_canvas* canvas, vs_object* v1, vs_object* v2, const char *title);

#endif
