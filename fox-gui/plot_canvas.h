
#include <fx.h>

#include <agg2/agg_rendering_buffer.h>

#include "plot_auto.h"
#include "vs_object.h"
#include "canvas.h"

class fx_canvas : public FXImage {
public:
  fx_canvas(FXApp *app, unsigned width, unsigned height) :
    FXImage(app, NULL, IMAGE_KEEP|IMAGE_OWNED|IMAGE_SHMI|IMAGE_SHMP, width, height),
    m_rbuf(NULL), m_canvas(NULL)
  { }

  virtual ~fx_canvas() {
    delete m_rbuf;
    delete m_canvas;
  }

  canvas* get_canvas() { return m_canvas; };
  
  virtual void create();

private:
  agg::rendering_buffer* m_rbuf;
  canvas* m_canvas;
};

class plot_canvas : public FXCanvas {
  FXDECLARE(plot_canvas)

  typedef plot_auto<vs_scaling_object, owner_management> plot_type;

public:
  plot_canvas(FXComposite* p,
	      FXObject* tgt=NULL, FXSelector sel=0, 
	      FXuint opts=FRAME_NORMAL,
	      FXint x=0, FXint y=0, FXint w=0, FXint h=0) :
    FXCanvas(p, tgt, sel, opts, x, y, w, h), m_plot(), m_img(0)
  {}

  virtual ~plot_canvas() { delete m_img; }

  plot_type& plot() { return m_plot; };

  void draw_plot(FXEvent*);
  void ensure_canvas_size(int ww, int hh);

  long on_cmd_paint(FXObject *, FXSelector, void *);

protected:
  plot_canvas(){};
private:
  plot_canvas(const plot_canvas&);
  plot_canvas &operator=(const plot_canvas&);

private:
  plot_type m_plot;
  fx_canvas* m_img;
};
