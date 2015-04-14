#ifndef FILMSTACK_WINDOW_H
#define FILMSTACK_WINDOW_H

#include <fx.h>

#include "stack.h"

class regress_pro;

class filmstack_window : public FXPacker {
    FXDECLARE(filmstack_window)

protected:
    filmstack_window() {};
private:
    filmstack_window(const filmstack_window&);
    filmstack_window &operator=(const filmstack_window&);

public:
    filmstack_window(stack_t *s, FXComposite *p, FXuint opts=0, FXint x=0, FXint y=0, FXint w=0, FXint h=0, FXint pl=DEFAULT_SPACING, FXint pr=DEFAULT_SPACING, FXint pt=DEFAULT_SPACING, FXint pb=DEFAULT_SPACING, FXint hs=DEFAULT_SPACING, FXint vs=DEFAULT_SPACING);
    virtual ~filmstack_window();

    virtual void create();

    regress_pro * regressProApp() { return (regress_pro *) getApp(); }

private:
    FXMenuPane *popupmenu;
    int current_layer;

public:
    /* The arguments FXObject *rcp with the two selector indicates an object
       that will be used as target to notify changes in film stack. */
    void set_target_stack_changes(FXObject *rcp, FXuint sel_change, FXuint sel_shift);

    void bind_new_filmstack(stack_t *s, bool notify = true);
    void update_values();

    long on_cmd_film_menu(FXObject*,FXSelector,void* ptr);
    long on_cmd_insert_layer(FXObject*,FXSelector,void* ptr);
    long on_cmd_replace_layer(FXObject*,FXSelector,void* ptr);
    long on_cmd_delete_layer(FXObject*,FXSelector,void* ptr);
    long on_cmd_edit_layer(FXObject*,FXSelector,void* ptr);
    long on_change_name(FXObject*,FXSelector,void* ptr);
    long on_change_thickness(FXObject*,FXSelector,void* ptr);
    long on_cmd_save_userlib(FXObject*,FXSelector,void* ptr);

    enum {
        ID_FILM_MENU = FXPacker::ID_LAST,
        ID_FILM_MENU_LAST = ID_FILM_MENU + 64,
        ID_FILM_NAME,
        ID_FILM_NAME_LAST = ID_FILM_NAME + 64,
        ID_FILM_THICKNESS,
        ID_FILM_THICKNESS_LAST = ID_FILM_THICKNESS + 64,
        ID_EDIT_LAYER,
        ID_DELETE_LAYER,
        ID_INSERT_LAYER,
        ID_REPLACE_LAYER,
        ID_SAVE_USERLIB,
        ID_LAST
    };

private:
    FXWindow *setup_stack_window(FXComposite *);
    void rebuild_stack_window(bool notify = true);
    void notify_stack_change();
    void notify_stack_shift(shift_info *);

    FXWindow *stack_window;
    FXMatrix *matrix;

    // If not NULL will send a message to the given target with the selectors below.
    FXObject *recipe_target;
    FXuint recipe_sel_change;
    FXuint recipe_sel_shift;

    stack_t *stack;
};

#endif
