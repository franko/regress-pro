#ifndef FILMSTACK_WINDOW_H
#define FILMSTACK_WINDOW_H

#include <fx.h>

class filmstack_window : public FXDialogBox {
    FXDECLARE(filmstack_window)

public:
protected:
    filmstack_window() {};
private:
    filmstack_window(const filmstack_window&);
    filmstack_window &operator=(const filmstack_window&);

public:
    filmstack_window(FXApp* a, FXuint opts=DECOR_ALL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,FXint hs=0,FXint vs=0);
    virtual ~filmstack_window();
};

#endif
