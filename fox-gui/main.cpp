
/* main.cpp
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

#include "regress_pro_window.h"
#include "regress_pro_testing.h"

const char *read_script_options(int argc, char** argv) {
    int i = 1;
    if (argc > i + 1 && strcmp(argv[i], "--script") == 0) {
        return argv[i+1];
    }
    return nullptr;
}

int main(int argc,char *argv[])
{
    regress_pro app;

    // Open display
    app.init(argc, argv);

    if (argc >= 2 && strcmp(argv[1], "-v") == 0) {
        printf("regress-pro version %d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
        exit(0);
    }

    const char *script = read_script_options(argc, argv);
    new FXToolTip(&app);
    // Main window
    regress_pro_window* window = new regress_pro_window(&app);

    app.create();
    if (script) {
        app.setScriptMode(true);
        testing_app testing(window);
        execute_script(script, testing);
        return 0;
    } else {
        window->show(PLACEMENT_SCREEN);
    }
    return app.run();
}
