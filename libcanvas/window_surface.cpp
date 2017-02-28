#include "window_surface.h"

#include "util/agg_color_conv_rgb8.h"

#include "fatal.h"
#include "colors.h"

namespace graphics {

window_surface::window_surface(const char* split_str):
m_img(), m_save_img(), m_window(0), m_canvas(0)
{
    split(split_str ? split_str : ".");
}

window_surface::~window_surface()
{
    delete m_canvas;
}

void window_surface::split(const char* split_str)
{
    m_part.parse(split_str);
    m_part.split();

    m_plots.clear();
    plot_ref empty;
    for (unsigned k = 0; k < m_part.get_slot_number(); k++)
        m_plots.add(empty);
}

bool window_surface::resize(unsigned ww, unsigned hh)
{
    m_save_img.clear();

    for (unsigned k = 0; k < plot_number(); k++)
        m_plots[k].have_save_img = false;

    if (likely(m_img.resize(ww, hh)))
    {
        delete m_canvas;
        m_canvas = new(std::nothrow) canvas(m_img, ww, hh, colors::white());
        return (m_canvas != NULL);
    }
    return false;
}

void window_surface::update_region_locked(image& img, const agg::rect_i& r)
{
    m_window->lock();
    m_window->update_region(img, r);
    m_window->unlock();
}

void window_surface::draw_image_buffer()
{
    for (unsigned k = 0; k < plot_number(); k++)
        render(k);
}

void window_surface::render(plot_ref& ref, const agg::rect_i& r)
{
    m_canvas->clear_box(r);
    if (ref.plot_ptr)
    {
        drawing_lock();
        ref.plot_ptr->draw(*m_canvas, r, &ref.inf);
        drawing_unlock();
    }
}

void window_surface::render(unsigned index)
{
    int canvas_width = get_width(), canvas_height = get_height();
    plot_ref& ref = m_plots[index];
    agg::rect_i area = m_part.rect(index, canvas_width, canvas_height);
    render(ref, area);
}

int window_surface::attach(plot* p, const char* slot_str)
{
    int index = m_part.get_slot_index(slot_str);
    if (index >= 0)
        m_plots[index].plot_ptr = p;
    return index;
}

bool window_surface::save_plot_image(unsigned index)
{
    int ww = get_width(), hh = get_height();
    if (unlikely(!m_save_img.ensure_size(ww, hh))) return false;

    agg::rect_i r = m_part.rect(index, ww, hh);
    image::copy_region(m_save_img, m_img, r);
    m_plots[index].have_save_img = true;
    return true;
}

bool window_surface::restore_plot_image(unsigned index)
{
    if (unlikely(!m_plots[index].have_save_img))
        fatal_exception("window_surface::restore_slot_image invalid restore image");

    int ww = get_width(), hh = get_height();
    agg::rect_i r = m_part.rect(index, ww, hh);
    image::copy_region(m_img, m_save_img, r);
    return true;
}

agg::rect_i window_surface::get_plot_area(unsigned index) const
{
    int canvas_width = get_width(), canvas_height = get_height();
    return m_part.rect(index, canvas_width, canvas_height);
}

agg::rect_i window_surface::get_plot_area(unsigned index, int width, int height) const
{
    return m_part.rect(index, width, height);
}

void
window_surface::render_all()
{
    for (unsigned k = 0; k < m_plots.size(); k++)
        render(k);
}

void
window_surface::draw_request()
{
    const agg::rect_i r(0, 0, get_width(), get_height());
    m_window->update_region(m_img, r);
}

void
window_surface::save_slot_image(unsigned index)
{
    save_plot_image(index);
}

void
window_surface::restore_slot_image(unsigned index)
{
    if (m_plots[index].have_save_img)
    {
        restore_plot_image(index);
    }
    else
    {
        render(index);
        save_plot_image(index);
    }
}

} /* namespace graphics */
