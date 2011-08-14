#include "BatchDialog.h"
#include "Strcpp.h"
#include "batch.h"
#include "grid-search.h"
#include "fit-params.h"

extern "C" {
  static int batch_foxgui_hook (void *data, float p, const char *msg);
};


// Map
FXDEFMAP(BatchDialog) BatchDialogMap[]={
  FXMAPFUNC(SEL_COMMAND, BatchDialog::ID_BROWSE, BatchDialog::onCmdBrowse),
  FXMAPFUNC(SEL_COMMAND, BatchDialog::ID_ACCEPT, BatchDialog::onCmdRun),
};

// Object implementation
FXIMPLEMENT(BatchDialog,FXDialogBox,BatchDialogMap,ARRAYNUMBER(BatchDialogMap));

BatchDialog::BatchDialog(FXWindow *w, struct fit_engine *xfit, struct seeds *sd)
  : FXDialogBox(w, "Batch Spectra Selection", DECOR_ALL, 0, 0, 320, 180),
    fit(xfit), seeds(sd)
{
  FXVerticalFrame *topfr = new FXVerticalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(topfr,"File Name Template",NULL,LABEL_NORMAL);
  FXHorizontalFrame *hfr1 = new FXHorizontalFrame(topfr,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  tfName = new FXTextField(hfr1,64,NULL,0,TEXTFIELD_NORMAL|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXButton(hfr1,"...",NULL,this,ID_BROWSE,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y);
  FXMatrix *matrix = new FXMatrix(topfr,3,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(matrix, "Starting Value", NULL, LAYOUT_FILL_ROW);
  new FXLabel(matrix, "Final Value", NULL, LAYOUT_FILL_ROW);
  new FXLabel(matrix, "Stride", NULL, LAYOUT_FILL_ROW);

  tfStart = new FXTextField(matrix,5,NULL,0,FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_INTEGER|LAYOUT_FILL_ROW);
  tfStart->setText("1");

  tfEnd = new FXTextField(matrix,5,NULL,0,FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_INTEGER|LAYOUT_FILL_ROW);
  tfEnd->setText("10");

  tfStep = new FXTextField(matrix,5,NULL,0,FRAME_SUNKEN|FRAME_THICK|TEXTFIELD_INTEGER|LAYOUT_FILL_ROW);
  tfStep->setText("1");

  new FXHorizontalSeparator(topfr,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  FXHorizontalFrame *btframe = new FXHorizontalFrame(topfr,LAYOUT_FILL_X|LAYOUT_RIGHT);
  new FXButton(btframe,"&Cancel",NULL,this,ID_CANCEL,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);
  new FXButton(btframe,"&Run",NULL,this,ID_ACCEPT,FRAME_THICK|FRAME_RAISED|LAYOUT_FILL_Y|LAYOUT_RIGHT,0,0,0,0,10,10,5,5);
}

long
BatchDialog::onCmdBrowse(FXObject*,FXSelector,void*)
{
  static const FXchar dat_patterns[] =
    "Fit Strategy (*.dat)"
    "\nAll Files (*)";

  FXFileDialog open(this,"Batch File Selection");
  open.setFilename(tfName->getText());
  open.setPatternList(dat_patterns);

  if(open.execute())
    {
      tfName->setText(open.getFilename());
    }      

  return 0;
}

int
batch_foxgui_hook (void *data, float p, const char *msg)
{
  FXWindow *win = (FXWindow *) data;
  FXApp *app = win->getApp();
  app->runModalWhileEvents(win);
  return 0;
}

long
BatchDialog::onCmdRun(FXObject*,FXSelector,void*)
{
  struct spectra_lst batch[1];
  str_t spectr_name;

  str_init (batch->name, 64);
  str_init (spectr_name, 64);

  if (batch_descr_parse (tfName->getText().text(), batch, 0))
    {
      FXMessageBox::information(this, MBOX_OK, "Batch Running",
				"Spectra Template Name is invalid.\n"
				"You should use characters \"###\" for "
				"number placeholders ");
      return 0;
    }

  batch->single_file = 0;
  batch->start  = FXIntVal(tfStart->getText(), 10);
  batch->stop   = FXIntVal(tfEnd->getText(),   10);
  batch->stride = FXIntVal(tfStep->getText(),  10);

  FXMessageBox waitbox(this, "Fit is running", "Please wait...", NULL, MBOX_QUIT_CANCEL);
  waitbox.create();
  waitbox.show(PLACEMENT_SCREEN);
  getApp()->refresh();
  getApp()->beginWaitCursor();

  const char *bname = strrchr (CSTR(batch->name), '/');

  str_t pname;
  str_init (pname, 32);
  FXString header, aux;
  aux.format("%%-%is ", bname ? strlen(bname) - 1 : 8);
  header.format(aux.text(), "Name");
  for (size_t j = 0; j < fit->parameters->number; j++)
    {
      fit_param_t *fp = & fit->parameters->values[j];
      get_param_name (fp, pname);
      aux.format("%-10s ", CSTR(pname));
      header.append(aux);
    }
  header.append("Chi^2\n");
  str_free (pname);

  result.format("Batch on Template:\n%s\n\nBatch Results:\n\n",
		tfName->getText().text());
  result.append(header);

  Str line;
  int k = batch->start;
  while (get_batch_filename (spectr_name, batch, &k))
    {
      struct spectrum *spectr = load_gener_spectrum (CSTR(spectr_name));
      double chisq;
      FXString row;

      if (spectr == NULL)
	continue;

      bname = strrchr (CSTR(spectr_name), DIR_SEPARATOR);
      if (bname)
	bname++;

      fit_engine_prepare (fit, spectr);

      lmfit_grid (fit, this->seeds, &chisq, NULL, NULL, LMFIT_PRESERVE_STACK,
		  batch_foxgui_hook, this);

      row.format("%s ", bname ? bname : "###");
      result.append(row);

      fit_engine_print_fit_results (fit, line.str(), 1);
      result.append(line.cstr());

      row.format(" %g\n", chisq);
      result.append(row);

      fit_engine_disable (fit);
      spectra_free (spectr);
    }

  getApp()->endWaitCursor();
  waitbox.hide();

  str_free (spectr_name);
  str_free (batch->name);

  getApp()->stopModal(this);
  hide();
  return 1;
}

void
BatchDialog::execute(FXString &res)
{
  create();
  show(PLACEMENT_SCREEN);
  getApp()->refresh();
  getApp()->runModalFor(this);
  res = this->result;
}
