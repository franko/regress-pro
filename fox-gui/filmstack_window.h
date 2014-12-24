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
    filmstack_window(FXApp* a, FXuint opts=DECOR_ALL,FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,FXint hs=0,FXint vs=0);
    virtual ~filmstack_window();

    virtual void create();

private:
    FXMenuPane *popupmenu;
    FXVerticalFrame *layersframe;
    int current_layer;

public:
    long on_stack_right_mouse(FXObject*,FXSelector,void* ptr);
    long on_cmd_insert_layer(FXObject*,FXSelector,void* ptr);
    long on_cmd_delete_layer(FXObject*,FXSelector,void* ptr);

    enum {
        ID_STACK = FXDialogBox::ID_LAST,
        ID_DELETE_LAYER,
        ID_INSERT_LAYER,
        ID_LAST
    };
};

#endif
