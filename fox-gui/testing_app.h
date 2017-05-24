#ifndef TESTING_APP_H
#define TESTING_APP_H

#include "str_cpp.h"

class regress_pro_window;

class testing_app {
public:
    testing_app(regress_pro_window *win): m_window(win) { }

    str_ptr execute(const str& command, const str& argument);

private:
    regress_pro_window *m_window;
};

#endif
