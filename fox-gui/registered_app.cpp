#include "registered_app.h"
#include "registration.h"

FXDEFMAP(registered_app) registered_app_map[]={
  FXMAPFUNC(SEL_COMMAND, registered_app::ID_REGISTER_MARK, registered_app::on_registration_mark),
  FXMAPFUNC(SEL_COMMAND, registered_app::ID_REGISTER,      registered_app::on_registration_enter),
  FXMAPFUNC(SEL_COMMAND, registered_app::ID_REGISTER_ASK,  registered_app::on_registration_ask),
};

// Object implementation
FXIMPLEMENT(registered_app, FXApp, registered_app_map,ARRAYNUMBER(registered_app_map));

registered_app::registered_app(const FXString& name,const FXString& vendor) :
  FXApp(name, vendor), m_reg_count(0), m_registered(false)
{
}

void
registered_app::create()
{
  FXApp::create();
  check_registration();
}

void
registered_app::check_registration()
{
  FXRegistry& reg = this->reg();

  const char *user_name  = reg.readStringEntry("Registration", "user", "no user");
  const char *user_email = reg.readStringEntry("Registration", "email", NULL);
  const char *key        = reg.readStringEntry("Registration", "key", NULL);

  if (!user_name || !user_email || !key)
    return;

  const int ver = REGISTRATION_VERSION;

  m_registered = registration_check (user_name, user_email, ver, key);
}

long
registered_app::on_registration_mark(FXObject*,FXSelector,void*)
{
  if (m_registered)
    return 0;

  m_reg_count ++;

  if (m_reg_count >= reg_count_limit)
    {
      show_registration_dialog();
      m_reg_count = 0;
      return 1;
    }

  return 0;
}

long
registered_app::on_registration_ask(FXObject*,FXSelector,void*)
{
  if (m_registered)
    show_registration_message();
  else
    show_registration_dialog();

  return 1;
}

static bool
user_email_is_correct(FXString& email, FXString& errmsg)
{
  FXString name = email.before('@'), domain = email.after('@');

  if (name.length() == 0)
    {
      if (email.find('@') < 0)
	errmsg = "does not contail the character '@'";
      else
	errmsg = "missing name before '@'";

      return false;
    }

  if (domain.length() == 0)
    {
      errmsg = "missing domain name after '@'";
      return false;
    }

  return true;
}

long
registered_app::on_registration_enter(FXObject*,FXSelector,void*)
{
  FXString user_name = m_user_name_entry->getText();
  FXString user_email = m_user_email_entry->getText();
  FXString key = m_key_entry->getText();

  if (user_name.length() < 8)
    {
      FXMessageBox::warning(this, MBOX_OK, "Regress Pro Registration",
			    "User name should be at least 8 character long");
      return 1;
    }

  FXString errmsg;
  if (! user_email_is_correct(user_email, errmsg))
    {
      FXMessageBox::warning(this, MBOX_OK, "Regress Pro Registration",
			    "Invalid email address: %s", errmsg.text());
      return 1;
    }

  if (key.length() == 0)
    {
      FXMessageBox::warning(this, MBOX_OK, "Regress Pro Registration",
			    "Please provide a non-empty registration key");
      return 1;
    }

  bool is_valid = registration_check (user_name.text(), user_email.text(), REGISTRATION_VERSION, key.text());

  if (is_valid)
    {
      FXRegistry& reg = this->reg();

      reg.writeStringEntry("Registration", "user",  user_name.text());
      reg.writeStringEntry("Registration", "email", user_email.text());
      reg.writeStringEntry("Registration", "key",   key.text());

      reg.write();

      FXMessageBox::information(this, MBOX_OK, "Regress Pro Registration",
				"Congratulation you have successfully registered\n"
				"Regress Pro for Windows, version %d.%d.%d.\n\n"
				"We would like to remember your that Regress Pro is Free Software.\n"
				"By registering your copy you support its development.\n"
				"Thank you very much.",
				VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH);

      m_registered = true;

      m_dialog->handle(this, FXSEL(SEL_COMMAND, FXDialogBox::ID_ACCEPT), NULL);
    }
  else
    {
      FXMessageBox::warning(this, MBOX_OK, "Regress Pro Registration",
			    "The Registration code is not correct");
    }

  return 1;
}

void
registered_app::show_registration_dialog()
{
  m_dialog = new FXDialogBox(this, "Regress Pro registration", DECOR_TITLE|DECOR_BORDER,0,0,0,0, 0,0,0,0, 0,0);
  FXVerticalFrame* side=new FXVerticalFrame(m_dialog,LAYOUT_SIDE_RIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 10,10,10,10, 0,0);
  new FXLabel(side,"R e g r e s s   P r o",NULL,JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_FILL_X);
  new FXHorizontalSeparator(side,SEPARATOR_LINE|LAYOUT_FILL_X);
  new FXLabel(side,FXString("\nPlease register your copy of Regress Pro for Windows.\n"),NULL,JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(side,FXStringFormat("Product Release Tag : %03i\n", REGISTRATION_VERSION),NULL,JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  FXMatrix *mat = new FXMatrix(side, 2, LAYOUT_FILL_X|LAYOUT_FILL_Y|MATRIX_BY_COLUMNS, 0, 0, 0, 0, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, 1, 1);

  new FXLabel(mat, "User name");
  m_user_name_entry = new FXTextField(mat, 36);

  new FXLabel(mat, "User email");
  m_user_email_entry = new FXTextField(mat, 36);

  new FXLabel(mat, "Key");
  m_key_entry = new FXTextField(mat, 36);
   
  FXHorizontalFrame *bfrm = new FXHorizontalFrame(side, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_X,0,0,0,0, 10,10,10,10, 0,0);
  new FXButton(bfrm,"Cancel",NULL,m_dialog,FXDialogBox::ID_CANCEL,FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,32,32,2,2);
  FXButton *button = new FXButton(bfrm,"&Register",NULL,this,registered_app::ID_REGISTER,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,32,32,2,2);
  button->setFocus();
  m_dialog->execute(PLACEMENT_OWNER);
}

void
registered_app::show_registration_message()
{
  FXDialogBox dlg(this, "Regress Pro registration", DECOR_TITLE|DECOR_BORDER,0,0,0,0, 0,0,0,0, 0,0);
  FXVerticalFrame* side=new FXVerticalFrame(&dlg,LAYOUT_SIDE_RIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 10,10,10,10, 0,0);
  new FXLabel(side,"R e g r e s s   P r o",NULL,JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_FILL_X);
  new FXHorizontalSeparator(side,SEPARATOR_LINE|LAYOUT_FILL_X);
  new FXLabel(side,FXString("\nThis copy of Regress Pro is registered to:\n"),NULL,JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  FXMatrix *mat = new FXMatrix(side, 2, LAYOUT_FILL_X|LAYOUT_FILL_Y|MATRIX_BY_COLUMNS, 0, 0, 0, 0, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, DEFAULT_SPACING, 1, 1);

  FXRegistry& reg = this->reg();

  const char *user_name  = reg.readStringEntry("Registration", "user", "no user");
  const char *user_email = reg.readStringEntry("Registration", "email", NULL);
  const char *key        = reg.readStringEntry("Registration", "key", NULL);

  new FXLabel(mat, "user name:");
  new FXLabel(mat, user_name, 0);

  new FXLabel(mat, "email:");
  new FXLabel(mat, user_email);

  new FXLabel(mat, "Key:");
  new FXLabel(mat, key);

  new FXLabel(side,FXStringFormat("Product Release Tag : %03i\n", REGISTRATION_VERSION),NULL,JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
   
  FXHorizontalFrame *bfrm = new FXHorizontalFrame(side, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_X,0,0,0,0, 10,10,10,10, 0,0);
  FXButton *button = new FXButton(bfrm,"&Ok",NULL,&dlg,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,32,32,2,2);
  button->setFocus();
  dlg.execute(PLACEMENT_OWNER);
}
