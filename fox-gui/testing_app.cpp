#include "testing_app.h"
#include "error-messages.h"
#include "regress_pro_window.h"

str_ptr testing_app::execute(const str& command, const str& argument) {
    if (command == "load_spectrum") {
        return m_window->load_spectrum(argument.text());
    } else if (command == "load_recipe") {
        return m_window->load_spectrum(argument.text());
    } else if (command == "fit") {
        return m_window->run_fit_command();
    } else {
        return new_error_message(TESTING_SCRIPT_ERROR, "Invalid script command: \"%s\".", command.text());
    }
    return nullptr;
}
