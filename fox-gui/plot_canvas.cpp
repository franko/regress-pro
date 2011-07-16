
#include "plot_canvas.h"
#include "colors.h"

// Map
FXDEFMAP(plot_canvas) plot_canvas_map[]={
  FXMAPFUNC(SEL_PAINT,  0, plot_canvas::on_cmd_paint),
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
}

void plot_canvas::ensure_canvas_size(int ww, int hh)
{
  if (! m_img)
    {
      prepare_image_buffer (ww, hh);
    }
  else if (m_img->getWidth() != ww || m_img->getHeight() != hh)
    {
      prepare_image_buffer (ww, hh);
    }
}

void
plot_canvas::draw_plot(FXEvent* event)
{
  int ww = getWidth(), hh = getHeight();

  ensure_canvas_size(ww, hh);

  if (m_canvas)
    {
      m_canvas->clear();
      m_plots.draw(m_canvas, ww, hh);
      m_img->render();

      FXDCWindow *dcwin = (event ? new FXDCWindow(this, event) : new FXDCWindow(this));
      dcwin->drawImage(m_img, 0, 0);
      delete dcwin;
    }
}

long
plot_canvas::on_cmd_paint(FXObject *, FXSelector, void *ptr)
{
  draw_plot((FXEvent*) ptr);
  return 1;
}
