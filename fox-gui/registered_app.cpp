#include "registered_app.h"
#include "registration.h"

FXDEFMAP(registered_app) registered_app_map[]={
  FXMAPFUNC(SEL_COMMAND, registered_app::ID_REGISTER_MARK, registered_app::on_registration_mark),
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
      bool reg = show_registration_message();
      /*      if (reg)
	      open_registration_dialog(); */
      m_reg_count = 0;
      return 1;
    }

  return 0;
}

bool
registered_app::show_registration_message()
{
  FXDialogBox dlg(this, "Regress Pro registration", DECOR_TITLE|DECOR_BORDER,0,0,0,0, 0,0,0,0, 0,0);
  FXVerticalFrame* side=new FXVerticalFrame(&dlg,LAYOUT_SIDE_RIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 10,10,10,10, 0,0);
  new FXLabel(side,"R e g r e s s   P r o",NULL,JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_FILL_X);
  new FXHorizontalSeparator(side,SEPARATOR_LINE|LAYOUT_FILL_X);
  new FXLabel(side,FXString("\nPlease register your copy of Regress Pro for Windows.\n"),NULL,JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXHorizontalFrame *bfrm = new FXHorizontalFrame(side, LAYOUT_SIDE_RIGHT|LAYOUT_FILL_X,0,0,0,0, 10,10,10,10, 0,0);
  new FXButton(bfrm,"Cancel",NULL,&dlg,FXDialogBox::ID_CANCEL,FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,32,32,2,2);
  FXButton *button = new FXButton(bfrm,"&OK",NULL,&dlg,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,32,32,2,2);
  button->setFocus();
  return dlg.execute(PLACEMENT_OWNER);
}
