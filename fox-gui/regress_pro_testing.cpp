
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

#include "regress_pro_testing.h"
#include "str_cpp.h"
#include "error-messages.h"
#include "str-util.h"
#include "lexer.h"

int execute_script(const char *script_filename, testing_app& app) {
    str script_text;
    str_loadfile(script_filename, &script_text);
    lexer_t *lex = lexer_new(script_text.text());
    str command, argument;
    while (lex->current.tk != TK_EOF) {
        if (lexer_ident(lex)) break;
        command = CSTR(lex->store);

        if (lexer_string(lex)) break;
        argument = CSTR(lex->store);

        str_ptr error_msg = app.execute(command, argument);
        if (error_msg) {
            fprintf(stderr, "%s\n", CSTR(error_msg));
            free_error_message(error_msg);
            break;
        }
    }
    lexer_free(lex);
    return 0;
}
