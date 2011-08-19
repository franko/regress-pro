#ifndef FITLEXER_H
#define FITLEXER_H

#include <ctype.h>
#include "defs.h"
#include "str.h"
#include "descr-util.h"
#include "descr.h"

__BEGIN_DECLS

struct symtab;

struct lex_stream {
  const char *text;
  int length;
  int index;
};

extern int      yylex               (void);
extern void     yyerror             (struct symtab *symtab, char const *s);
extern void     lexer_init          (const char *text);
extern str_ptr  get_lex_string      (lex_string_t *lstr);
extern int      lex_string_equal    (lex_string_t *s, const char *ref);

#define LEX_STRING_EQUAL(s,ref) lex_string_equal (s,ref)

extern struct lex_stream stream[1];
extern YYLTYPE yylloc;

__END_DECLS

#endif
