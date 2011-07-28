%{

/* descr.y
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

#include <ctype.h>
#include <string.h>

#include "descr-util.h"
#include "interp.h"
#include "descr.h"
#include "fitlexer.h"
#include "error-messages.h"

#define YYERROR_VERBOSE

#if defined (DEBUG_REGRESS) && YYDEBUG
#define YYPRINT(file, type, value)   yyprint (file, type, value)
static void yyprint (FILE *file, int type, YYSTYPE value);
#endif

%}
%locations
%union {
  int vide;
  lex_string_t string[1];
  double number;
  struct value * val;
  struct value_lst * vallist;
  struct pair * field;
  struct pair_lst * fieldlist;
}
%token USE
%token SET
%token <number> NUMBER
%token <string> STRING STRINGID
%type <val> value record
%type <vallist> list csvs
%type <field> field
%type <fieldlist> fields
%type <vide> assignments
%type <vide> directive

%parse-param {struct symtab *symtab}

%destructor { value_free ($$); } value record
%destructor { value_list_free ($$); } list csvs
%destructor { pair_free ($$); } field
%destructor { pair_list_free ($$); } fields

%%

input: assignments              { }
;

record: STRINGID '{' fields '}' { $$ = build_rec_from_pairlist ($1, $3); }
      | STRINGID NUMBER         { $$ = build_rec_from_value ($1, value_from_double ($2)); }
      | STRINGID STRING         { $$ = build_rec_from_value ($1, value_from_string ($2, VAL_TP_STRING)); }
; 

field: STRINGID '=' value	{ $$ = build_pair ($1, $3); }
;

fields: field			{ $$ = pairlist_add_pair ($1, NULL); }
      | fields ',' field	{ $$ = pairlist_add_pair ($3, $1); }
;

list:   '[' ']'                 { $$ = NULL; }
      | '[' csvs ']'            { $$ = $2; }
;

csvs: value	                { $$ = list_add_value ($1, NULL);; }
    | csvs ',' value	        { $$ = list_add_value ($3, $1); }
;

value:  record
	| STRING	{ $$ = value_from_string ($1, VAL_TP_STRING); }
	| NUMBER	{ $$ = value_from_double ($1); }
        | STRINGID      { $$ = value_from_string ($1, VAL_TP_VARREF); }
	| list		{ $$ = value_from_list   ($1); }
;

directive: USE STRINGID STRINGID  { if (setup_use_directive (symtab, $2, $3)) {
	                            notify_script_err_msg (SCRIPT_ERROR,
                                      @3.first_line, @3.last_line,
                                      "error in directive"); YYERROR; }
                                }
;

setting: SET STRINGID value { if (set_config_value (symtab, $2, $3)) {
	                            notify_script_err_msg (SCRIPT_ERROR,
                                      @3.first_line, @3.last_line,
                                      "error in setting"); YYERROR; }
                                }
;

assignments: /* empty */  { }
        | assignments field { if (parse_assignment (symtab, $2)) {
                                notify_script_err_msg (SCRIPT_ERROR,
				@2.first_line, @2.last_line,
                                "error in definition of \"%s\"",
				CSTR($2->key)); pair_free ($2); YYERROR; }
                              pair_free ($2); }
        | assignments directive { }
        | assignments setting { }
;
%%

#if defined (DEBUG_REGRESS) && YYDEBUG
void
yyprint (FILE *file, int type, YYSTYPE value)
{
  if (type == STRING || type == STRINGID)
    {
      char buffer[64];
      int nbc = value.string->length < 64 ? value.string->length : 63;
      memcpy (buffer, stream->text + value.string->index, nbc);
      buffer[nbc] = 0;
      fprintf (file, " %s", buffer);
    }
  else if (type == NUMBER)
    fprintf (file, " %lf", value.number);
}
#endif
