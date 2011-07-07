
/* regress_pro_window.cpp
 * 
 * Copyright (C) 2005-2011 Francesco Abbate
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include <gsl/gsl_vector.h>

#include "regress_pro_window.h"
#include "DispersDialog.h"
#include "BatchDialog.h"
#include "interactive_fit.h"
#include "Strcpp.h"
#include "error-messages.h"
#include "fit-engine.h"
#include "multi-fit-engine.h"
#include "lmfit-multi.h"
#include "fit-params.h"
#include "spectra.h"
#include "symtab.h"
#include "grid-search.h"
#include "str.h"
#include "str-util.h"
#include "dispers-library.h"
#include "spectra-path.h"
#include "disp_chooser.h"
#include "disp_fit_window.h"

static float timeval_subtract (struct timeval *x, struct timeval *y);

// Map
FXDEFMAP(regress_pro_window) regress_pro_window_map[]={
  FXMAPFUNC(SEL_PAINT,   regress_pro_window::ID_CANVAS, regress_pro_window::onCmdPaint),
  FXMAPFUNC(SEL_UPDATE,  regress_pro_window::ID_CANVAS, regress_pro_window::onUpdCanvas),
  FXMAPFUNC(SEL_UPDATE,  regress_pro_window::ID_SCRIPT_TEXT, regress_pro_window::onUpdScript),
  FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_ABOUT,  regress_pro_window::onCmdAbout),
  FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_REGISTER,  regress_pro_window::onCmdRegister),
  FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_LOAD_SCRIPT, regress_pro_window::onCmdLoadScript),
  FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_SAVE_SCRIPT, regress_pro_window::onCmdSaveScript),
  FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_SAVEAS_SCRIPT, regress_pro_window::onCmdSaveAsScript),
  FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_LOAD_SPECTRA, regress_pro_window::onCmdLoadSpectra),
  FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_DISP_PLOT, regress_pro_window::onCmdPlotDispers),
  FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_DISP_OPTIM, regress_pro_window::onCmdDispersOptim),
  FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_RUN_FIT, regress_pro_window::onCmdRunFit),
  FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_INTERACTIVE_FIT, regress_pro_window::onCmdInteractiveFit),
  FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_RUN_MULTI_FIT, regress_pro_window::onCmdRunMultiFit),
  FXMAPFUNC(SEL_COMMAND, regress_pro_window::ID_RUN_BATCH, regress_pro_window::onCmdRunBatch),
  };


// Object implementation
FXIMPLEMENT(regress_pro_window,FXMainWindow,regress_pro_window_map,ARRAYNUMBER(regress_pro_window_map));


const FXchar regress_pro_window::patterns_fit[] =
  "Fit Strategy (*.fit)"
  "\nAll Files (*)";
const FXchar regress_pro_window::patterns_spectr[] =
  "Fit Strategy (*.dat)"
  "\nAll Files (*)";

const FXHiliteStyle regress_pro_window::tstyles[] = {
  {FXRGB(255,255,255), FXRGB(255,0,0), 0, 0, 0, 0, 0, 0}};
  


// Make some windows
regress_pro_window::regress_pro_window(elliss_app* a) 
 : FXMainWindow(a,"Regress Pro",NULL,&a->appicon,DECOR_ALL,20,20,700,460),
   spectrum(NULL), stack_result(NULL), scriptFile("untitled"),
   spectrFile("untitled"), batchFileId("untitled####.dat"), m_elliss_app(a) {

  // Menubar
  menubar=new FXMenuBar(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  statusbar=new FXStatusBar(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|FRAME_RAISED|STATUSBAR_WITH_DRAGCORNER);

  // Script menu
  filemenu=new FXMenuPane(this);
  new FXMenuCommand(filemenu,"&Load",NULL,this,ID_LOAD_SCRIPT);
  new FXMenuCommand(filemenu,"&Save",NULL,this,ID_SAVE_SCRIPT);
  new FXMenuCommand(filemenu,"Save As",NULL,this,ID_SAVEAS_SCRIPT);
  new FXMenuCommand(filemenu,"&Quit\tCtl-Q",NULL,getApp(),FXApp::ID_QUIT);
  new FXMenuTitle(menubar,"&Script",NULL,filemenu);

  // Script menu
  spectrmenu = new FXMenuPane(this);
  new FXMenuCommand(spectrmenu,"&Load Spectra",NULL,this,ID_LOAD_SPECTRA);
  new FXMenuTitle(menubar,"S&pectra",NULL,spectrmenu);

  // Dispersion menu
  dispmenu = new FXMenuPane(this);
  new FXMenuCommand(dispmenu, "&Plot Dispersion",NULL,this,ID_DISP_PLOT);
  new FXMenuCommand(dispmenu, "Dispersion Optimize",NULL,this,ID_DISP_OPTIM);
  new FXMenuTitle(menubar,"&Dispersion",NULL,dispmenu);

  // Fit menu
  fitmenu = new FXMenuPane(this);
  new FXMenuCommand(fitmenu, "&Run Fitting",NULL,this,ID_RUN_FIT);
  new FXMenuCommand(fitmenu, "&Interactive Fit",NULL,this,ID_INTERACTIVE_FIT);
  new FXMenuCommand(fitmenu, "Run &Multiple Fit",NULL,this,ID_RUN_MULTI_FIT);
  new FXMenuCommand(fitmenu, "Run &Batch",NULL,this,ID_RUN_BATCH);
  new FXMenuTitle(menubar,"Fittin&g",NULL,fitmenu);

  helpmenu = new FXMenuPane(this);
  new FXMenuCommand(helpmenu, "&Register", NULL, this, ID_REGISTER);
  new FXMenuCommand(helpmenu, "&About", NULL, this, ID_ABOUT);
  new FXMenuTitle(menubar, "&Help", NULL, helpmenu, LAYOUT_RIGHT);

  // Container
  FXHorizontalFrame *cont = new FXHorizontalFrame(this,LAYOUT_FILL_X|LAYOUT_FILL_Y);

  tabbook = new FXTabBook(cont,NULL,0,PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_RIGHT);

  // First item is a list
  tabscript = new FXTabItem(tabbook,"&Script",NULL);
  FXHorizontalFrame *lf = new FXHorizontalFrame(tabbook,FRAME_THICK|FRAME_RAISED);
  FXHorizontalFrame *bf = new FXHorizontalFrame(lf,FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);
  scripttext = new FXText(bf,this,ID_SCRIPT_TEXT,TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  scripttext->setStyled(TRUE);
  scripttext->setHiliteStyles(tstyles);
  scriptfont = new FXFont(getApp(), "Monospace", 10);
  scripttext->setFont(scriptfont);

  new FXTabItem(tabbook,"&Fit Results",NULL);
  lf = new FXHorizontalFrame(tabbook,FRAME_THICK|FRAME_RAISED);
  bf = new FXHorizontalFrame(lf,FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);
  resulttext = new FXText(bf,NULL,0,TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  resulttext->setFont(scriptfont);

  tabplot = new FXTabItem(tabbook,"&Plot Result",NULL);
  lf = new FXHorizontalFrame(tabbook,FRAME_THICK|FRAME_RAISED);
  bf = new FXHorizontalFrame(lf,FRAME_THICK|FRAME_SUNKEN|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);
  plotcanvas = new FXCanvas(bf,this,ID_CANVAS,LAYOUT_FILL_X|LAYOUT_FILL_Y);

  plotkind = SYSTEM_UNDEFINED;

  spectrPlot1 = new plot(a);
  spectrPlot2 = new plot(a);

  isPlotModified = true;

  symbol_table_init (this->symtab);

  init_class_list ();

  dispers_library_init ();

  m_title_dirty = true;
  m_title_modified = false;
}


// Create image
void
regress_pro_window::create(){
  FXMainWindow::create();
  scriptfont->create();
}

void
regress_pro_window::plotCanvas(FXDCWindow *dc)
{
  int ww = plotcanvas->getWidth(), hh = plotcanvas->getHeight();

  switch (plotkind)
    {
    case SYSTEM_REFLECTOMETER:
      spectrPlot1->draw(dc, ww, hh);
      break;
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
      spectrPlot1->draw(dc, ww, hh / 2);
      spectrPlot2->draw(dc, ww, hh / 2, 0, hh / 2);
      break;
    default:
      dc->setForeground(FXRGB(255,255,255));
      dc->fillRectangle(0, 0, ww, hh);
    }
}

// Command from chart
long
regress_pro_window::onCmdPaint(FXObject *, FXSelector, void *ptr)
{
  if (isPlotModified)
    {
      FXDCWindow dc(plotcanvas);
      plotCanvas(&dc);
    }
  else
    {
      FXDCWindow dc(plotcanvas, (FXEvent*) ptr);
      plotCanvas(&dc);
    }
  isPlotModified = false;
  return 1;
}

long
regress_pro_window::onUpdCanvas(FXObject*, FXSelector, void *)
{
  if (isPlotModified)
    {
      FXDCWindow dc(plotcanvas);
      plotCanvas(&dc);
      isPlotModified = false;
      return 1;
    }
  return 0;
}

long
regress_pro_window::onUpdScript(FXObject*, FXSelector, void *)
{
  bool is_mod = scripttext->isModified();

  if (m_title_dirty || (is_mod != m_title_modified))
    {
      bool is_reg = get_elliss_app()->is_registered();

      FXString filename = scriptFile.rafter(DIR_SEPARATOR);
      FXString pathname = scriptFile.rbefore(DIR_SEPARATOR);
      FXString flag(is_mod ? "*" : "");
      FXString appname(is_reg ? "Regress Pro" : "(UNREGISTERED)");

      this->setTitle(flag + filename + " - " + pathname + " - " + appname);
      m_title_dirty = false;
      m_title_modified = is_mod;
      return 1;
    }
  return 0;
}

long
regress_pro_window::onCmdLoadScript(FXObject*,FXSelector,void *)
{
  reg_check_point(this);

  FXFileDialog open(this,"Open Script");
  open.setFilename(scriptFile);
  open.setPatternList(patterns_fit);

  if(open.execute())
    {
      scriptFile = open.getFilename();
      Str script_text;

      if (str_loadfile (scriptFile.text(), script_text.str()) != 0)
	return 0;

#ifdef ENABLE_SCRIPT_REL_PATH
      str_init_from_c (script_file, scriptFile.text());
      str_dirname (this->symtab->env->script_dir, script_file, DIR_SEPARATOR);
      str_free (script_file);
#endif

      scripttext->setText(script_text.cstr());
      setFitStrategy(script_text.cstr());
      scripttext->setModified(FALSE);

      m_title_dirty = true;
    }

  return 1;
}

FXbool
regress_pro_window::saveScriptAs (const FXString& save_as)
{
  FILE *f = fopen (save_as.text(), "w");

  if (f == NULL)
    {
      FXMessageBox::information(this, MBOX_OK, "Script save",
				"Cannot write file %s\n", scriptFile.text());
      return false;
    }

  if (fputs (scripttext->getText().text(), f) == EOF)
    {
      FXMessageBox::information(this, MBOX_OK, "Script save",
				"Cannot write file %s\n", scriptFile.text());
      fclose (f);
      return false;
    }

  fputc ('\n', f);
  fclose (f);

  scripttext->setModified(FALSE);

  m_title_dirty = true;

  return true;
}

long
regress_pro_window::onCmdSaveAsScript(FXObject*,FXSelector,void *)
{
  reg_check_point(this);

  FXFileDialog open(this, "Save Script As");
  open.setFilename(scriptFile);
  open.setPatternList(patterns_fit);

  if(open.execute())
    {
      FXString new_filename = open.getFilename();
      if (saveScriptAs(new_filename))
	scriptFile = new_filename;
      return 1;
    }

  return 0;
}

long
regress_pro_window::onCmdSaveScript(FXObject*,FXSelector,void *)
{
  reg_check_point(this);

  saveScriptAs(scriptFile);
  return 1;
}

long
regress_pro_window::onCmdLoadSpectra(FXObject*,FXSelector,void *)
{
  reg_check_point(this);

  FXFileDialog open(this,"Open Spectra");
  open.setFilename(spectrFile);
  open.setPatternList(patterns_spectr);

  if(open.execute())
    {
      spectrFile = open.getFilename();

      if (this->spectrum)
	spectra_free (this->spectrum);

      this->spectrum = load_gener_spectrum (spectrFile.text());

      if (this->spectrum == NULL)
	FXMessageBox::information(this, MBOX_OK, "Spectra loading",
				  "Cannot load spectra %s", spectrFile.text());

      return 1;
    }

  return 0;
}

long
regress_pro_window::onCmdPlotDispers(FXObject*,FXSelector,void*)
{
  if (this->stack_result)
    {
      DispersDialog dialog(this, this->stack_result);
      dialog.execute();
      return 1;
    }
  return 0;
}

long
regress_pro_window::onCmdDispersOptim(FXObject*,FXSelector,void*)
{
  reg_check_point(this);

  updateFitStrategy();

  if (! this->symtab)
    return 1;

  struct disp_fit_engine *fit = disp_fit_engine_new ();

  if (disp_chooser (getApp(), this->symtab, fit))
    {
      disp_fit_window *fitwin = new disp_fit_window(get_elliss_app(), fit);
      fitwin->create();
      fitwin->show(FX::PLACEMENT_SCREEN);
    }
  else
    {
      disp_fit_engine_free (fit);
    }


  return 1;
}

long
regress_pro_window::onCmdAbout(FXObject *, FXSelector, void *)
{
  FXDialogBox about(this,"About Regress Pro",DECOR_TITLE|DECOR_BORDER,0,0,0,0,
		    0,0,0,0, 0,0);
  FXVerticalFrame* side=new FXVerticalFrame(&about,LAYOUT_SIDE_RIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 10,10,10,10, 0,0);
  new FXLabel(side,"R e g r e s s   P r o",NULL,JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_FILL_X);
  new FXHorizontalSeparator(side,SEPARATOR_LINE|LAYOUT_FILL_X);
  new FXLabel(side,FXStringFormat("\nRegress Pro, version %d.%d.%d.\n\n" "Regress Pro is a scientific / industrial software to perform regression\nanalysis of measurement data coming from spectroscopic\nellipsometers or reflectometers.\n" "Regress Pro uses the FOX Toolkit version %d.%d.%d.\nCopyright (C) 2005-2011 Francesco Abbate (francesco.bbt@gmail.com).\n",VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH,FOX_MAJOR,FOX_MINOR,FOX_LEVEL),NULL,JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXButton *button=new FXButton(side,"&OK",NULL,&about,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,32,32,2,2);
  button->setFocus();
  about.execute(PLACEMENT_OWNER);
  return 1;
}

long
regress_pro_window::onCmdRegister(FXObject *, FXSelector, void *)
{
  reg_form(this);
  m_title_dirty = true;
  return 1;
}

// Clean up
regress_pro_window::~regress_pro_window() {
  delete scriptfont;
  delete filemenu;
  delete spectrmenu;
  delete dispmenu;
  delete fitmenu;
  delete helpmenu;
  delete spectrPlot1;
  delete spectrPlot2;

  if (this->spectrum)
    spectra_free (this->spectrum);

  if (this->stack_result)
    stack_free (this->stack_result);

  symbol_table_clean (this->symtab);
  symbol_table_free  (this->symtab);

  clean_error_msgs ();
}

long
regress_pro_window::onCmdRunBatch(FXObject*,FXSelector,void *)
{
  struct seeds *seeds;
  struct fit_engine *fit;

  reg_check_point(this);

  fit = build_fit_engine (this->symtab, &seeds);

  if (fit == NULL)
    return 0;

  BatchDialog batch(this, fit, seeds);
  batch.setFilename(batchFileId);

  FXString result;
  batch.execute(result);
  resulttext->setText(result);
  resulttext->setModified(TRUE);

  batchFileId = batch.getFilename();

  fit_engine_free (fit);

  return 1;
}

long
regress_pro_window::onCmdRunMultiFit(FXObject*,FXSelector,void *)
{
  struct multi_fit_engine *fit;
  Str analysis;
  struct {
    struct seeds *common;
    struct seeds *individual;
  } seeds;

  updateFitStrategy();

  fit = build_multi_fit_engine (this->symtab,
				&seeds.common, &seeds.individual);

  if (fit == NULL)
    return 0;

  Str fit_error_msgs;
  ProgressInfo progress(this->getApp(), this);

  lmfit_multi (fit, seeds.common, seeds.individual,
	       analysis.str(), fit_error_msgs.str(),
	       LMFIT_GET_RESULTING_STACK, 
	       process_foxgui_events, &progress);

  progress.hide();

  if (fit_error_msgs.length() > 0)
    {
      FXMessageBox::information(this, MBOX_OK, "Multiple Fit messages",
				fit_error_msgs.cstr());
      clean_error_msgs ();
    }

  FXString text_fit_result;

  Str fp_results;
  multi_fit_engine_print_fit_results (fit, fp_results.str());
  text_fit_result.append(fp_results.cstr());

  text_fit_result.append(analysis.cstr());

  resulttext->setText(text_fit_result);
  resulttext->setModified(TRUE);

  multi_fit_engine_disable (fit);
  multi_fit_engine_free (fit);

  return 1;
}

void
regress_pro_window::updateFitStrategy()
{
  if (scripttext->isModified())
    setFitStrategy (scripttext->getText().text());
}

bool
regress_pro_window::check_spectrum(const char *context)
{
  if (spectrum == NULL)
    {
      FXMessageBox::information(this, MBOX_OK, context,
				"Please load a spectra before.");
      return false;
    }

  return true;
}

long
regress_pro_window::onCmdRunFit(FXObject*,FXSelector,void *)
{
  if (! check_spectrum("Fitting"))
    return 0;

  reg_check_point(this);

  updateFitStrategy();

  double chisq;
  Str analysis;
  struct seeds *seeds;
  struct fit_engine *fit;

  fit = build_fit_engine (this->symtab, &seeds);

  if (fit == NULL)
    {
      reportErrors();
      return 0;
    }

  fit_engine_prepare (fit, this->spectrum, 1);

  Str fit_error_msgs;
  ProgressInfo progress(this->getApp(), this);

  lmfit_grid (fit, seeds, &chisq, analysis.str(), fit_error_msgs.str(),
	      LMFIT_GET_RESULTING_STACK,
	      process_foxgui_events, & progress);

  progress.hide();

  if (fit_error_msgs.length() > 0)
    {
      FXMessageBox::information(this, MBOX_OK, "Fit messages",
				fit_error_msgs.cstr());
      clean_error_msgs ();
    }

  FXString fitresult, row;

  /* name of the fit script */
  row.format("%s :\n", scriptFile.text());
  fitresult.append(row);

  /* fit parameters results */
  Str fit_parameters_results;
  fit_engine_print_fit_results (fit, fit_parameters_results.str(), 0);
  fitresult.append(fit_parameters_results.cstr());

  /* final chi square obtained */
  row.format("Residual Chisq/pt: %g\n", chisq);
  fitresult.append(row);

  /* covariance matrix analysis */
  fitresult.append("\n");
  fitresult.append(analysis.cstr());

  resulttext->setText(fitresult);
  resulttext->setModified(TRUE);

  fit_engine_restore_spectr (fit);

  this->plotkind = fit->system_kind;

  struct spectrum *gensp = generate_spectrum (fit);
  switch (fit->system_kind)
    {
    case SYSTEM_REFLECTOMETER:
      {
	agg::path_storage *esp = new agg::path_storage;
	agg::path_storage *gsp = new agg::path_storage;
	refl_spectrum_path (fit->spectr, esp);
	refl_spectrum_path (gensp, gsp);

	spectrPlot1->clear();
	spectrPlot1->add(esp, FXRGB(255,0,0));
	spectrPlot1->add(gsp);
	spectrPlot1->set_title("Reflectivity");
	break;
      }
    case SYSTEM_ELLISS_AB:
    case SYSTEM_ELLISS_PSIDEL:
      {
	agg::path_storage *esp1 = new agg::path_storage;
	agg::path_storage *esp2 = new agg::path_storage;
	agg::path_storage *gsp1 = new agg::path_storage;
	agg::path_storage *gsp2 = new agg::path_storage;

	elliss_spectrum_path (fit->spectr, esp1, esp2);
	elliss_spectrum_path (gensp, gsp1, gsp2);

	spectrPlot1->clear();
	spectrPlot1->add(esp1, FXRGB(255,0,0));
	spectrPlot1->add(gsp1);
	spectrPlot1->set_title(fit->system_kind == SYSTEM_ELLISS_AB ?	\
			       "SE alpha" : "Tan(Psi)");

	spectrPlot2->clear();
	spectrPlot2->add(esp2, FXRGB(255,0,0));
	spectrPlot2->add(gsp2);
	spectrPlot2->set_title(fit->system_kind == SYSTEM_ELLISS_AB ?	\
			       "SE beta" : "Cos(Delta)");
	break;
      }
    default:
      /* */;
    }

  spectra_free (gensp);

  if (this->stack_result)
    stack_free (this->stack_result);

  this->stack_result = stack_copy (fit->stack);

  fit_engine_disable (fit);
  fit_engine_free (fit);

  isPlotModified = true;

  getApp()->endWaitCursor();

  return 1;
}

long
regress_pro_window::onCmdInteractiveFit(FXObject*,FXSelector,void*)
{
  if (! check_spectrum("Fitting"))
    return 0;

  reg_check_point(this);

  updateFitStrategy();

  struct seeds *seeds;
  struct fit_engine *fit = build_fit_engine (symtab, &seeds);

  if (fit == NULL)
    {
      reportErrors();
      return 0;
    }

  interactive_fit *fitwin = new interactive_fit(get_elliss_app(), fit, spectrum);
  fitwin->create();
  fitwin->show(FX::PLACEMENT_SCREEN);
  return 1;
}

FXbool
regress_pro_window::setFitStrategy(const char *script_text)
{
  cleanScriptErrors();

  if (parse_strategy (this->symtab, script_text) != 0)
    {
      int fline, lline;
      str_t errmsg;
      str_init (errmsg, 128);
      
      if (get_script_error_region (&fline, &lline) == 0)
	setErrorRegion(fline, lline);

      get_errors_list (errmsg);

      FXMessageBox::information(this, MBOX_OK, "Script parsing",
				"The parsing of the script has been"
				" unsuccessful :\n%s", CSTR(errmsg));

      str_free (errmsg);
      return false;
    }

  return true;
}

void
regress_pro_window::reportErrors()
{
  str_t errmsg;
  str_init (errmsg, 128);
  get_errors_list (errmsg);
  FXMessageBox::information(this, MBOX_OK, "Script parsing",
			    "The parsing of the script has been"
			    " unsuccessful :\n%s", CSTR(errmsg));
  str_free (errmsg);
}

void
regress_pro_window::cleanScriptErrors ()
{
  scripttext->changeStyle(0, scripttext->getLength(), 0);
}

void
regress_pro_window::setErrorRegion (int sl, int el)
{
  int cl = 1, ns = 0, next;
  FXString text(scripttext->getText());
  const char *base = text.text();

  while (cl < sl)
    {
      const char *ptr = strchr (base, '\n');
      if (! ptr)
	break;
      ptr ++;
      ns += ptr - base;
      base = ptr;
      cl ++;
    }

  if (cl < sl)
    return;

  next = 0;
  while (cl <= el)
    {
      const char *ptr = strchr (base, '\n');
      if (! ptr)
	break;
      ptr ++;
      next += ptr - base;
      base = ptr;
      cl ++;
    }
  
  scripttext->changeStyle(ns, next, 1);
}

float
timeval_subtract (struct timeval *x, struct timeval *y)
{
  float result = y->tv_sec - x->tv_sec;
  result += (y->tv_usec - x->tv_usec) / 1.0E6;
  return result;
}

int
process_foxgui_events (void *data, float progr, const char *msg)
{
  const float wait_time = 0.4, progress_thresold = 0.5;
  ProgressInfo *info = (ProgressInfo *) data;
  struct timeval current[1];

  if (gettimeofday (current, NULL) != 0)
    return 0;

  float diff = timeval_subtract (info->start, current);
  const int multfactor = 4096;

  if (info->dialog == NULL)
    {
      if (diff < wait_time || progr > progress_thresold)
	return 0;

      info->dialog = new FXProgressDialog(info->window,
					  "Fit is running",
					  "Please wait...",
					  PROGRESSDIALOG_CANCEL);

      info->dialog->setTotal(multfactor);
      info->dialog->setBarStyle(PROGRESSBAR_PERCENTAGE);
      info->dialog->create();
      info->dialog->show(PLACEMENT_SCREEN);

      info->dialog->setProgress((int) (progr * multfactor));
      info->dialog->repaint();

      info->app->beginWaitCursor();
      
      return 0;
    }
  
  if (info->dialog)
    {
      info->dialog->setProgress((int) (progr * multfactor));
      info->dialog->repaint();
      if (msg)
	info->dialog->setMessage(msg);

      info->app->runModalWhileEvents(info->dialog);

      return (info->dialog->shown() ? 0 : 1);
    }

  return 0;
}
