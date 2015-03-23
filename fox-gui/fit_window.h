
/* fit_window.h
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

#ifndef FIT_WINDOW_H
#define FIT_WINDOW_H

#include <fx.h>

#include "fit_panel.h"

class fit_window : public FXMainWindow {
    FXDECLARE(fit_window)

    FXMenuBar *menubar;
    FXStatusBar *statusbar;
    FXMenuPane *fitmenu, *plotmenu;
    fit_panel *m_fit_panel;
protected:
    fit_window() {};
private:
    fit_window(const fit_window&);
    fit_window &operator=(const fit_window&);

public:
    fit_window(fit_manager* fit,FXApp* a,const FXString& name,FXIcon *ic=NULL,FXIcon *mi=NULL,FXuint opts=DECOR_ALL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=0,FXint pr=0,FXint pt=0,FXint pb=0,FXint hs=0,FXint vs=0);
    virtual ~fit_window();
};

#endif
