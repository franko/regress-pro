
/* fit_window.cpp
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

#include "fit_window.h"

// Map
FXDEFMAP(fit_window) fit_window_map[]= {
    FXMAPFUNC(SEL_COMMAND, fit_window::ID_DELETE, fit_window::onCmdHide),
};

// Object implementation
FXIMPLEMENT(fit_window,FXDialogBox,fit_window_map,ARRAYNUMBER(fit_window_map));

fit_window::fit_window(fit_manager* fit, FXWindow *win, const FXString& name, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs)
    : FXDialogBox(win, name, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{
    menubar = new FXMenuBar(this, LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
    statusbar = new FXStatusBar(this, LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|FRAME_RAISED|STATUSBAR_WITH_DRAGCORNER);

    m_fit_panel = new fit_panel(fit, this, LAYOUT_FILL_X|LAYOUT_FILL_Y);

    fitmenu = new FXMenuPane(this);
    new FXMenuCommand(fitmenu, "&Run", NULL, m_fit_panel, fit_panel::ID_RUN_FIT);
    new FXMenuTitle(menubar, "&Fit", NULL, fitmenu);

    plotmenu = new FXMenuPane(this);
    new FXMenuCommand(plotmenu, "&Auto Scale", NULL, m_fit_panel, fit_panel::ID_PLOT_SCALE);
    new FXMenuTitle(menubar, "&Plot", NULL, plotmenu);
}

fit_window::~fit_window()
{
    delete fitmenu;
    delete plotmenu;
}
