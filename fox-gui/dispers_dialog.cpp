#include "dispers_dialog.h"
#include "dispers_win.h"

// Map
FXDEFMAP(dispers_dialog) dispers_dialog_map[]={
  FXMAPFUNC(SEL_COMMAND, dispers_dialog::ID_PLOT, dispers_dialog::on_cmd_plot),
  FXMAPFUNC(SEL_COMMAND, dispers_dialog::ID_LAYER_CHANGE, dispers_dialog::on_cmd_layer_change),
};

// Object implementation
FXIMPLEMENT(dispers_dialog,FXDialogBox,dispers_dialog_map,ARRAYNUMBER(dispers_dialog_map));

dispers_dialog::dispers_dialog(FXWindow *w, struct stack *s)
  : FXDialogBox(w,"Select Dispersion", DECOR_ALL, 0, 0, 320, 100)
{
  this->stack = s;
  
  FXVerticalFrame *mfr  = new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXHorizontalFrame *hfr1 = new FXHorizontalFrame(mfr,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  layerSpinner = new FXSpinner(hfr1,1,this,ID_LAYER_CHANGE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FIX_WIDTH,0,0,60,0);
  dispLabel = new FXLabel(hfr1,"",NULL,LABEL_NORMAL);
  new FXHorizontalSeparator(mfr,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  FXHorizontalFrame *btframe = new FXHorizontalFrame(mfr,LAYOUT_FILL_X|LAYOUT_RIGHT);
  new FXButton(btframe,"&Cancel",NULL,this,ID_CANCEL,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);
  new FXButton(btframe,"&Plot",NULL,this,ID_PLOT,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);

  layerSpinner->setRange(0, s->nb - 1);
  on_cmd_layer_change(NULL, 0, NULL);
}

long
dispers_dialog::on_cmd_layer_change(FXObject*, FXSelector, void *)
{
  layer = layerSpinner->getValue();

  if (layer < 0 || layer >= this->stack->nb)
    return 0;

  const char *name = CSTR(this->stack->disp[layer]->name);

  FXString dname;
  if (strcmp (name, "") == 0)
    dname.format("Dipersion layer %i", layer);
  else
    dname = name;

  dispLabel->setText(dname);

  return 1;
}

long
dispers_dialog::on_cmd_plot(FXObject*, FXSelector, void *)
{
  if (layer < 0 || layer >= this->stack->nb)
    return 0;

  dispers_win dwin(this, this->stack->disp[layer]);
  dwin.execute();

  return 1;
}
