
#include <fx.h>

#include <agg2/agg_rendering_buffer.h>

#include "plot_auto.h"
#include "agg_plot_array.h"
#include "vs_object.h"
#include "canvas.h"

class plot_canvas : public FXCanvas {
  FXDECLARE(plot_canvas)

public:
  typedef newplot::plot_auto<vs_object, owner_management> plot_type;

  plot_canvas(FXComposite* p,
	      FXObject* tgt=NULL, FXSelector sel=0, 
	      FXuint opts=FRAME_NORMAL,
	      FXint x=0, FXint y=0, FXint w=0, FXint h=0) :
    FXCanvas(p, tgt, sel, opts, x, y, w, h), m_canvas(0), m_img(0)
  {}

  virtual ~plot_canvas() { 
    delete m_img; 
    delete m_canvas;
  }

  void add(plot_type* plot) { m_plots.add(plot); }

  long on_cmd_paint(FXObject *, FXSelector, void *);

protected:
  plot_canvas(){};
private:
  plot_canvas(const plot_canvas&);
  plot_canvas &operator=(const plot_canvas&);

private:
  void draw_plot(FXEvent*);
  void prepare_image_buffer(int ww, int hh);
  void ensure_canvas_size(int ww, int hh);

  newplot::plot_array<plot_type, newplot::vertical_layout> m_plots;

  agg::rendering_buffer m_rbuf;

  canvas* m_canvas;
  FXImage* m_img;
};
