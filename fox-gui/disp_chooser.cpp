
#include <assert.h>

#include "dispers-library.h"

#include "disp_chooser.h"

class disp_iter {
  struct symtab *m_symtab;

  enum iter_class_e {
    iter_symtab,
    iter_library,
  };

  str_t m_name_buffer;

  enum iter_class_e m_iter_sel;
  struct assign *m_iter;
  int m_lib_iter;

  const char *assign_iter() {
    while (m_iter = symbol_table_next (m_symtab, m_iter))
      {
	if (m_iter->value->type == TL_TYPE_DISP)
	  break;
      }
    return (m_iter ? CSTR(m_iter->name) : 0);
  }

  const char *library_iter() {
    const char* name;
    if (dispers_library_get(m_lib_iter ++, &name))
      {
	str_printf (m_name_buffer, "lib:%s", name);
	return CSTR(m_name_buffer);
      }
    return NULL;
  }

public:
  disp_iter(struct symtab *s) : m_symtab(s) {
    str_init (m_name_buffer, 32);
  }

  ~disp_iter() {
    str_free (m_name_buffer);
  }

  void reset() { 
    m_iter_sel = iter_symtab;
    m_iter = 0;
  }

  const char *start() {
    reset();
    return next();
  }

  const char *next()  {
    if (m_iter_sel == iter_symtab)
      {
	const char* name = assign_iter();
	if (name)
	  return name;

	m_iter_sel = iter_library;
	m_lib_iter = 0;

	return library_iter();
      }
    else if (m_iter_sel == iter_library)
      {
	return library_iter();
      }

    return 0;
  }

  disp_t* get(const char* name)
  {
    if (strncmp (name, "lib:", 4) == 0)
      {
	const char* disp_name = name + 4;
	return dispers_library_search (disp_name);
      }

    str_t name_view;
    str_init_view (name_view, name);
    return (disp_t*) retrieve_parsed_object (m_symtab, TL_TYPE_DISP, name_view);
  }

  int length()
  {
    int n = 0;
    for (const char *nm = start(); nm; nm = next())
      n ++;
    return n;
  }
};

class disp_chooser_win : public FXDialogBox {
  FXDECLARE(disp_chooser_win)

protected:
  disp_chooser_win(){};
private:
  disp_chooser_win(const disp_chooser_win&);
  disp_chooser_win &operator=(const disp_chooser_win&);

public:
  disp_chooser_win(FXApp *app, disp_iter& disp_list);

  long onCmdAccept(FXObject *, FXSelector, void *ptr);

  disp_t*   ref_disp() const { return m_ref;   }
  disp_t* model_disp() const { return m_model; }

private:
  FXComboBox* m_ref_combo;
  FXComboBox* m_mod_combo;

  disp_t* m_ref;
  disp_t* m_model;

  disp_iter* m_disp_list;
};

// Map
FXDEFMAP(disp_chooser_win) disp_chooser_win_map[]={
  FXMAPFUNC(SEL_COMMAND, FXDialogBox::ID_ACCEPT, disp_chooser_win::onCmdAccept),
};

// Object implementation
FXIMPLEMENT(disp_chooser_win,FXDialogBox,disp_chooser_win_map,ARRAYNUMBER(disp_chooser_win_map));

disp_chooser_win::disp_chooser_win (FXApp *app, disp_iter& disp_list) :
  FXDialogBox(app, "Choose Dispersions", DECOR_TITLE|DECOR_BORDER),
  m_ref(0), m_model(0), m_disp_list(&disp_list)
{
  // Bottom buttons
  FXHorizontalFrame * buttons= new FXHorizontalFrame(this, LAYOUT_SIDE_BOTTOM|FRAME_NONE|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH); // ,0,0,0,0,40,40,20,20);

  // Separator
  new FXHorizontalSeparator(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|SEPARATOR_GROOVE);

  FXMatrix* mat = new FXMatrix(this, 2, LAYOUT_SIDE_TOP|LAYOUT_FILL_Y|MATRIX_BY_COLUMNS);

  new FXLabel(mat, "Reference");

  int nb_disp = disp_list.length();

  m_ref_combo = new FXComboBox(mat, 10, NULL, 0, COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK);
  m_ref_combo->setNumVisible(nb_disp);
  for (const char *nm = disp_list.start(); nm; nm = disp_list.next())
    m_ref_combo->appendItem(nm);

  new FXLabel(mat, "Dispersion model");

  m_mod_combo = new FXComboBox(mat, 10, NULL, 0, COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK);
  m_mod_combo->setNumVisible(nb_disp);
  for (const char *nm = disp_list.start(); nm; nm = disp_list.next())
    m_mod_combo->appendItem(nm);

  // Accept
  new FXButton(buttons, "&Accept", NULL, this, FXDialogBox::ID_ACCEPT, BUTTON_DEFAULT|BUTTON_INITIAL|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT|LAYOUT_CENTER_Y);

  // Cancel
  new FXButton(buttons, "&Cancel", NULL, this, FXDialogBox::ID_CANCEL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT|LAYOUT_CENTER_Y);
}

long
disp_chooser_win::onCmdAccept(FXObject *, FXSelector, void *ptr)
{
  FXString ref_name = m_ref_combo->getText();
  FXString mod_name = m_mod_combo->getText();

  m_ref   = m_disp_list->get (ref_name.text());
  m_model = m_disp_list->get (mod_name.text());

  assert (m_ref && m_model);

  if (m_ref == m_model)
    {
      FXMessageBox::warning(getApp(), MBOX_OK, "Dispersion choice",
			    "Please select two different dispersions");
      return 1;
    }

  if (disp_get_number_of_params (m_model) == 0)
    {
      FXMessageBox::warning(getApp(), MBOX_OK, "Dispersion choice",
			    "The dispersion '%s' does not contain any parameter",
			    m_mod_combo->getText().text());
      return 1;
    }

  getApp()->stopModal(this, TRUE);
  hide();
  return 1;
}

bool
disp_chooser (FXApp *app, struct symtab *symtab, struct disp_fit_engine *fit)
{
  disp_iter disp_list(symtab);

  if (disp_list.length() == 0)
    {
      FXMessageBox::warning(app, MBOX_OK, "Dispersion choice",
			    "Please define some dispersions in the script");
      return false;
    }

  disp_chooser_win dialog(app, disp_list);

  if (dialog.execute(PLACEMENT_OWNER))
    {
      fit->ref_disp   = dialog.ref_disp();
      fit->model_disp = dialog.model_disp();
      return true;
    }

  return false;
}
