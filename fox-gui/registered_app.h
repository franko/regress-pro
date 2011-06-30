#ifndef REGISTERED_APP_H
#define REGISTERED_APP_H

#include <fx.h>

class registered_app : public FXApp {
  FXDECLARE(registered_app)

  enum { reg_count_limit = 8 };

public:
  registered_app(const FXString& name,const FXString& vendor);

  virtual void create();

  void check_registration();

  long on_registration_mark(FXObject*,FXSelector,void*);

  enum {
    ID_REGISTER_MARK = FXApp::ID_LAST,
    ID_LAST
    };

protected:
  registered_app() {};
private:
  registered_app(const registered_app&);
  registered_app &operator=(const registered_app&);

protected:
  bool show_registration_message();

private:
  int m_reg_count;
  bool m_registered;
};

#endif
