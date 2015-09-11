
#include "plot_canvas.h"
#include "colors.h"

// Map
FXDEFMAP(plot_canvas) plot_canvas_map[]= {
    FXMAPFUNC(SEL_PAINT,  0, plot_canvas::on_cmd_paint),
    FXMAPFUNC(SEL_UPDATE, 0, plot_canvas::on_update),
    FXMAPFUNC(SEL_CLIPBOARD_GAINED, 0, plot_canvas::on_clipboard_gained),
    FXMAPFUNC(SEL_CLIPBOARD_LOST, 0, plot_canvas::on_clipboard_lost),
    FXMAPFUNC(SEL_CLIPBOARD_REQUEST, 0, plot_canvas::on_clipboard_request),
};

// Object implementation
FXIMPLEMENT(plot_canvas,FXCanvas,plot_canvas_map,ARRAYNUMBER(plot_canvas_map));

void plot_canvas::prepare_image_buffer(int ww, int hh)
{
    delete m_img;
    m_img = new FXImage(getApp(), NULL, IMAGE_KEEP|IMAGE_OWNED|IMAGE_SHMI|IMAGE_SHMP, ww, hh);
    m_img->create();

    agg::int8u* buf = (agg::int8u*) m_img->getData();
    unsigned width = ww, height = hh;
    unsigned stride = - width * sizeof(FXColor);

    m_rbuf.attach(buf, width, height, stride);
    m_canvas = new canvas(m_rbuf, width, height, colors::white);

    plainTextDataType = getApp()->registerDragType("plain/text");
}

void plot_canvas::ensure_canvas_size(int ww, int hh)
{
    if(! m_img) {
        prepare_image_buffer(ww, hh);
    } else if(m_img->getWidth() != ww || m_img->getHeight() != hh) {
        prepare_image_buffer(ww, hh);
    }
}

void
plot_canvas::draw_plot(FXEvent* event)
{
    int ww = getWidth(), hh = getHeight();

    /* To prevent a bug related to request of creating 1x1 paint. */
    if (ww < 16 || hh < 16) return;

    ensure_canvas_size(ww, hh);

    if(m_canvas) {
        m_canvas->clear();
        m_plots.draw(m_canvas, ww, hh);
        m_img->render();

        FXDCWindow *dc = (event ? new FXDCWindow(this, event) : new FXDCWindow(this));
        dc->drawImage(m_img, 0, 0);
        delete dc;
    }

    m_dirty_flag = false;
}

long
plot_canvas::on_update(FXObject *, FXSelector, void *)
{
    if(m_dirty_flag) {
        draw_plot(NULL);
        return 1;
    }
    return 0;
}

long
plot_canvas::on_cmd_paint(FXObject *, FXSelector, void *ptr)
{
    FXEvent* ev = (FXEvent*) ptr;
    draw_plot(dirty() ? NULL : ev);
    return 1;
}

void plot_canvas::acquire_clipboard()
{
    acquireClipboard(&plainTextDataType, 1);
}

long
plot_canvas::on_clipboard_gained(FXObject *, FXSelector, void *)
{
    vector_objects<plot_content> *content_list = new vector_objects<plot_content>();
    for (unsigned i = 0; i < plot_number(); i++) {
        plot_content *content = new plot_content();
        get_data(i, &content->lines);
        content->title = plot(i)->title();
        content_list->add(content);
    }
    delete m_clipboard_content;
    m_clipboard_content = content_list;
    return 1;
}

long
plot_canvas::on_clipboard_lost(FXObject *sender, FXSelector sel, void *ptr)
{
    FXCanvas::onClipboardLost(sender, sel, ptr);
    delete m_clipboard_content;
    m_clipboard_content = 0;
    return 1;
}

long
plot_canvas::on_clipboard_request(FXObject *sender, FXSelector sel, void *ptr)
{
    if(FXCanvas::onClipboardRequest(sender, sel, ptr)) return 1;

    FXEvent *event = (FXEvent*) ptr;
    if(event->target == plainTextDataType) {
        str text;
        for (unsigned k = 0; k < m_clipboard_content->size(); k++) {
            plot_content *pc = m_clipboard_content->at(k);
            for (unsigned p = 0; p < pc->lines.size(); p++) {
                cpair_table *table = pc->lines.tables[p];
                text += "x\ty\n";
                for (unsigned i = 0; i < table->size(); i++) {
                    text += str::format("%f\t%f\n", table->at(i).x, table->at(i).y);
                }
                text += "\n\n";
            }
        }

        FXuchar *data;
        FXuint len = text.length + 1;
        FXMALLOC(&data, FXuchar, len);
        memcpy(data, text.heap, len);

        // Give the array to the system!
        setDNDData(FROM_CLIPBOARD, event->target, data, len);

        // Return 1 because it was handled here
        return 1;
    }

  // Return 0 to signify we haven't dealt with it yet; a derived
  // class from MyWidget may yet give it another try ...
  return 0;
}

static void
add_new_plot_raw(plot_canvas* canvas, vs_object* ref, vs_object* model,
                 const char *title)
{
    agg::rgba8 red(220,0,0);
    agg::rgba8 blue(0,0,220);

    plot_canvas::plot_type *p = new plot_canvas::plot_type();
    p->set_title(title);
    p->pad_mode(true);

    if(ref) {
        p->add(ref,   red, true);
    }
    if(model) {
        p->add(model, blue, true);
    }
    p->commit_pending_draw();

    canvas->add(p);
}

void add_new_simple_plot(plot_canvas* canvas, vs_object* v, const char *title)
{
    add_new_plot_raw(canvas, v, 0, title);
}

void add_new_plot(plot_canvas* canvas, vs_object* v1, vs_object* v2,
                  const char *title)
{
    add_new_plot_raw(canvas, v1, v2, title);
}

