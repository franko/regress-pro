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
    bool is_registered() const {
        return m_registered;
    }

    long on_registration_enter(FXObject*,FXSelector,void*);
    long on_registration_ask(FXObject*,FXSelector,void*);
    long on_registration_mark(FXObject*,FXSelector,void*);

    enum {
        ID_REGISTER_MARK = FXApp::ID_LAST,
        ID_REGISTER_ASK,
        ID_REGISTER,
        ID_LAST
    };

protected:
    registered_app() {};
private:
    registered_app(const registered_app&);
    registered_app &operator=(const registered_app&);

protected:
    void show_registration_message();
    void show_registration_dialog();

private:
    int m_reg_count;
    bool m_registered;

    FXDialogBox *m_dialog;
    FXTextField *m_user_name_entry;
    FXTextField *m_user_email_entry;
    FXTextField *m_key_entry;
};

#endif
