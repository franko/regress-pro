
#include "plot_canvas.h"
#include "colors.h"

void fx_canvas::create()
{
  FXImage::create();

  agg::int8u* buf = (agg::int8u*) this->getData();
  unsigned width = getWidth(), height = getHeight();
  unsigned stride = - width * sizeof(FXColor);

  m_rbuf = new agg::rendering_buffer(buf, width, height, stride);
  m_canvas = new canvas(*m_rbuf, width, height, colors::white);
}

// Map
FXDEFMAP(plot_canvas) plot_canvas_map[]={
  FXMAPFUNC(SEL_PAINT,  0, plot_canvas::on_cmd_paint),
};

// Object implementation
FXIMPLEMENT(plot_canvas,FXCanvas,plot_canvas_map,ARRAYNUMBER(plot_canvas_map));

void plot_canvas::ensure_canvas_size(int ww, int hh)
{
  if (! m_img)
    {
      m_img = new fx_canvas(getApp(), ww, hh);
      m_img->create();
    }
  else if (m_img->getWidth() != ww || m_img->getHeight() != hh)
    {
      fx_canvas *new_img = new fx_canvas(getApp(), ww, hh);
      new_img->create();
      delete m_img;
      m_img = new_img;
    }
}

void
plot_canvas::draw_plot(FXEvent* event)
{
  int ww = getWidth(), hh = getHeight();

  ensure_canvas_size(ww, hh);

  canvas* canvas = m_img->get_canvas();
  if (canvas)
    {
      agg::trans_affine mt(double(ww), 0.0, 0.0, double(hh), 0.0, 0.0);
      canvas->clear();
      m_plot.draw(*canvas, mt);

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
