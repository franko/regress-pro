#include "disp_chooser.h"

#include "Strcpp.h"

class disp_iter {
  struct symtab *m_symtab;
  struct assign *m_iter;

  const char *iter(struct assign *it) {
    m_iter = it;
    while (m_iter = symbol_table_next (m_symtab, m_iter))
      {
	if (m_iter->value->type == TL_TYPE_DISP)
	  break;
      }
    return (m_iter ? CSTR(m_iter->name) : 0);
  }

public:
  disp_iter(struct symtab *s) : m_symtab(s), m_iter(0) {};

  const char *start() { return iter(0); }
  const char *next()  { return iter(m_iter); }
};

bool
disp_chooser (FXApp *app, struct symtab *symtab, struct disp_fit_engine *fit)
{
  disp_iter dispit(symtab);

  int nb_disp = 0;
  for (const char *nm = dispit.start(); nm; nm = dispit.next())
    nb_disp ++;

  FXDialogBox dialog(app, "Choose Dispersions", DECOR_TITLE|DECOR_BORDER);

  // Bottom buttons
  FXHorizontalFrame * buttons= new FXHorizontalFrame(&dialog, LAYOUT_SIDE_BOTTOM|FRAME_NONE|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH,0,0,0,0,40,40,20,20);

  // Separator
  new FXHorizontalSeparator(&dialog, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|SEPARATOR_GROOVE);

  FXMatrix* mat = new FXMatrix(&dialog, 2, LAYOUT_SIDE_TOP|LAYOUT_FILL_Y|MATRIX_BY_COLUMNS);

  new FXLabel(mat, "Reference");

  FXComboBox* refcombo = new FXComboBox(mat, nb_disp, NULL, 0, COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK);
  refcombo->setNumVisible(nb_disp);
  for (const char *nm = dispit.start(); nm; nm = dispit.next())
    refcombo->appendItem(nm);

  new FXLabel(mat, "Dispersion model");

  FXComboBox* modcombo = new FXComboBox(mat, nb_disp, NULL, 0, COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK);
  modcombo->setNumVisible(nb_disp);
  for (const char *nm = dispit.start(); nm; nm = dispit.next())
    modcombo->appendItem(nm);

  // Accept
  new FXButton(buttons, "&Accept", NULL, &dialog, FXDialogBox::ID_ACCEPT, BUTTON_DEFAULT|BUTTON_INITIAL|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT|LAYOUT_CENTER_Y);

  // Cancel
  new FXButton(buttons, "&Cancel", NULL, &dialog, FXDialogBox::ID_CANCEL, BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT|LAYOUT_CENTER_Y);

  if (dialog.execute(PLACEMENT_OWNER))
    {
      FXString s = refcombo->getText();
      Str rs((const char *) s.text());
      fit->ref_disp = (disp_t*) retrieve_parsed_object (symtab, TL_TYPE_DISP, rs.str());

      s = modcombo->getText();
      Str ms((const char *) s.text());
      fit->model_disp = (disp_t*) retrieve_parsed_object (symtab, TL_TYPE_DISP, ms.str());

      if (!fit->ref_disp)
	{
	  FXMessageBox::warning(app, MBOX_OK, "Dispersion choice",
				"The requested reference dispersion '%s' is not valid",
				rs.cstr());
	  return false;
	}

      if (!fit->model_disp)
	{
	  FXMessageBox::warning(app, MBOX_OK, "Dispersion choice",
				"The requested model dispersion '%s' is not valid",
				ms.cstr());
	  return false;
	}

      return true;
    }

  return false;
}
