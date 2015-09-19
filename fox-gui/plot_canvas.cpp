
#include <fx.h>
#include <FXPNGImage.h>

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

FXDragType plot_canvas::urilist_type = 0;
FXDragType plot_canvas::html_type = 0;
FXDragType plot_canvas::bmp_type = 0;
FXDragType plot_canvas::png_type = 0;

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
}

void plot_canvas::create()
{
    FXCanvas::create();
    FXApp *app = getApp();
    if (!urilist_type) urilist_type = app->registerDragType("text/uri-list");
    if (!html_type) html_type = app->registerDragType("text/html");
    if (!bmp_type) bmp_type = app->registerDragType(FXBMPImage::mimeType);
    if (!png_type) png_type = app->registerDragType(FXPNGImage::mimeType);
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

void plot_canvas::acquire_clipboard_as_text()
{
    m_clipboard_request_text = true;
    FXDragType types[3];
    types[0] = stringType;
    types[1] = textType;
    types[2] = html_type;
    acquireClipboard(types, 3);
}

void plot_canvas::acquire_clipboard_as_image()
{
    m_clipboard_request_text = false;
    FXDragType types[3];
    types[0] = FXWindow::imageType;
    types[1] = bmp_type;
    types[2] = png_type;
    acquireClipboard(types, 3);
}

long
plot_canvas::on_clipboard_gained(FXObject *, FXSelector, void *)
{
    this->free_clipboard_text();
    this->free_clipboard_image();

    if (m_clipboard_request_text) {
        vector_objects<plot_content> *content_list = new vector_objects<plot_content>();
        for (unsigned i = 0; i < plot_number(); i++) {
            plot_content *content = new plot_content();
            get_data(i, &content->lines);
            content->title = plot(i)->title();
            content_list->add(content);
        }
        m_clipboard_content = content_list;
    } else {
        int width = getWidth(), height = getHeight();
        image *img = new image(width, height);
        if (!img->data) {
            delete img;
            return 0;
        }
        const unsigned stride = - width * sizeof(FXColor);
        agg::rendering_buffer rbuf;
        rbuf.attach((agg::int8u*) img->data, width, height, stride);
        canvas can(rbuf, width, height, colors::white);

        can.clear();
        m_plots.draw(&can, width, height);
        m_clipboard_image = img;
    }
    return 1;
}

long
plot_canvas::on_clipboard_lost(FXObject *sender, FXSelector sel, void *ptr)
{
    FXCanvas::onClipboardLost(sender, sel, ptr);
    this->free_clipboard_text();
    this->free_clipboard_image();
    return 1;
}

struct column_info {
    enum column_e { X, Y };
    cpair_table *table;
    column_e column;
    str header;
    column_info(): table(0) {}
    column_info(cpair_table *t, column_e c, const str& h): table(t), column(c), header(h) {}

    bool is_null() const { return (table == 0); }

    float value(unsigned i) const {
        cpair& pair = table->at(i);
        return (column == X ? pair.x : pair.y);
    }
};

bool columns_are_equal(const column_info& a, const column_info& b)
{
    if (!a.table || !b.table) return false;
    if (a.table->size() != b.table->size()) return false;
    if (a.header != b.header) return false;
    for (unsigned i = 0; i < a.table->size(); i++) {
        if (fabs(a.value(i) - b.value(i)) > 1e-20) {
            return false;
        }
    }
    return true;
}

template <class Writer>
str write_table(agg::pod_bvector<column_info>& layout_info)
{
    str text;
    text += Writer::begin_fragment();

    text += Writer::begin_header_fragment();
    for (unsigned j = 0; j < layout_info.size(); j++) {
        text += Writer::write_cell(layout_info[j].header.text());
        if (j + 1 < layout_info.size()) {
            text += Writer::cell_separator();
        }
    }
    text += Writer::end_header_fragment();

    text += Writer::begin_body_fragment();
    for (unsigned i = 0; true; i++) {
        bool miss = true;
        text += Writer::begin_row_fragment();
        for (unsigned j = 0; j < layout_info.size(); j++) {
            str value;
            if (layout_info[j].table && i < layout_info[j].table->size()) {
                text += Writer::write_cell(layout_info[j].value(i));
                miss = false;
            } else {
                text += Writer::write_cell("");
            }
            if (j + 1 < layout_info.size()) {
                text += Writer::cell_separator();
            }
        }
        text += Writer::end_row_fragment();
        if (miss) break;
    }
    text += Writer::end_body_fragment();
    text += Writer::end_fragment();
    return text;
}

struct HtmlWriter {
    static const char *begin_fragment() { return "<table>"; }
    static const char *end_fragment() { return "</table>"; }
    static const char *begin_header_fragment() { return "<thead>"; }
    static const char *end_header_fragment() { return "</thead>"; }
    static const char *begin_body_fragment() { return "<tbody>"; }
    static const char *end_body_fragment() { return "</tbody>"; }
    static const char *begin_row_fragment() { return "<tr>"; }
    static const char *end_row_fragment() { return "<tr>"; }
    static const char *cell_separator() { return ""; }
    static str write_cell(float v) { return str::format("<td>%f</td>", v); }
    static str write_cell(const char *s) { return str::format("<td>%s</td>", s); }
};

struct TextWriterBase {
    static const char *begin_fragment() { return ""; }
    static const char *end_fragment() { return ""; }
    static const char *begin_header_fragment() { return ""; }
    static const char *end_header_fragment() { return "\n"; }
    static const char *begin_body_fragment() { return ""; }
    static const char *end_body_fragment() { return ""; }
    static const char *begin_row_fragment() { return ""; }
    static const char *end_row_fragment() { return "\n"; }
    static str write_cell(float v) { return str::format("%f", v); }
    static str write_cell(const char *s) { return str(s); }
};

struct TextWriter : TextWriterBase {
    static const char *cell_separator() { return "\t"; }
};

struct CsvWriter : TextWriterBase {
    static const char *cell_separator() { return ","; }
};

void compute_table_layout(agg::pod_bvector<column_info>& layout_info, vector_objects<plot_content> *content)
{
    for (unsigned k = 0; k < content->size(); k++) {
        plot_content *pc = content->at(k);
        column_info x_ref;
        for (unsigned p = 0; p < pc->lines.size(); p++) {
            cpair_table *table = pc->lines.tables[p];
            str header = str::format("%s/%s", pc->title.text(), pc->lines.names[p]->text());
            column_info x_col(table, column_info::X, "wavelength");
            if (!columns_are_equal(x_col, x_ref)) {
                layout_info.add(x_col);
            }
            layout_info.add(column_info(table, column_info::Y, header));
            if (x_ref.is_null()) {
                x_ref = x_col;
            }
        }
        if (k + 1 < content->size()) {
            layout_info.add(column_info());
        }
    }
}

long
plot_canvas::on_clipboard_request(FXObject *sender, FXSelector sel, void *ptr)
{
    if(FXCanvas::onClipboardRequest(sender, sel, ptr)) return 1;

    FXDragType target = ((FXEvent*) ptr)->target;
    if (m_clipboard_content && (target == stringType || target == textType || target == html_type)) {
        str text;
        agg::pod_bvector<column_info> layout_info;
        compute_table_layout(layout_info, m_clipboard_content);

        if (target == html_type) {
            text = write_table<HtmlWriter>(layout_info);
        } else {
            text = write_table<TextWriter>(layout_info);
        }

        FXuchar *data;
        FXuval len = text.length + 1;
        FXMALLOC(&data, FXuchar, len);
        memcpy(data, text.heap, len);

        setDNDData(FROM_CLIPBOARD, target, data, len);
        return 1;
    } else if (m_clipboard_image && (target == imageType || target == bmp_type || target == png_type)) {
        FXMemoryStream ms;
        ms.open(FXStreamSave, NULL);
        bool success;
        image *img = m_clipboard_image;
        if (target == imageType || target == bmp_type) {
            success = FX::fxsaveBMP(ms, img->data, img->width, img->height);
        } else {
            success = FX::fxsavePNG(ms, img->data, img->width, img->height);
        }

        if (success) {
            FXuchar *data;
            FXuval len;
            ms.takeBuffer(data, len);
            if (target == imageType) {
                /* We subtract for the BMP buffer the size of:

                typedef struct tagBITMAPFILEHEADER {
                    WORD    bfType;
                    DWORD   bfSize;
                    WORD    bfReserved1;
                    WORD    bfReserved2;
                    DWORD   bfOffBits;
                } BITMAPFILEHEADER;

                which is equal to 14. */
                const unsigned BMPHS = 14;
                setDNDData(FROM_CLIPBOARD, target, data + BMPHS, len - BMPHS);
            } else {
                setDNDData(FROM_CLIPBOARD, target, data, len);
            }
        }
        return 1;
    }
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
