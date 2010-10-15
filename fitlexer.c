
/*
  $Id: fitlexer.c,v 1.6 2006/12/26 18:15:50 francesco Exp $
 */

#include <string.h>
#include <assert.h>
#include "fitlexer.h"
#include "descr-util.h"
#include "error-messages.h"

struct lex_stream stream[1];

int
yylex ()
{
#define MYGETC(i) ((i)->index >= (i)->length ? EOF : (i)->text[(i)->index ++])
#define MYUNGETC(i) ((i)->index--)
#define SET_INDEX_START(str,i) (str)->index = (i)->index
#define SET_INDEX_END(str,i) (str)->length = (i)->index - (str)->index
  int c;

  do {
    c = MYGETC(stream);
    if (c == '\n')
      {
	yylloc.first_line ++;
	yylloc.last_line ++;
	continue;
      }
  } while (c == ' ' || c == '\t' || c == '\n' || c == '\015');

  if (isdigit (c) || c == '.' || c == '-')
    {
      double v;
      const char *start;
      char *tail;
      MYUNGETC(stream);
      start = stream->text + stream->index;
      v = strtod (start, &tail);
      if (tail == start)
	return MYGETC(stream);
      stream->index = tail - stream->text;
      yylval.number = v;
      return NUMBER;
    }
  else if (c == '"')
    {
      lex_string_t *s = yylval.string;
      SET_INDEX_START(s,stream);
      while ((c = MYGETC(stream)))
	if (c == '"' || c == EOF)
	  break;
      if (c == EOF)
	return 0;
      SET_INDEX_END(s,stream);
      s->length --;
      return STRING;
    }
  else if (c == '#')
    {
      do
	c = MYGETC(stream);
      while (! (c == '\n' || c == EOF));

      if (c == '\n')
	{
	  yylloc.first_line ++;
	  yylloc.last_line ++;
	}

      return yylex ();
    }
  else if (isalpha (c))
    {
      lex_string_t *s = yylval.string;
      MYUNGETC(stream);
      SET_INDEX_START(s,stream);
      while ((c = MYGETC(stream)))
	if (!isalnum(c) && c != '_')
	  break;
      if (c != EOF)
	MYUNGETC(stream);
      SET_INDEX_END(s,stream);

      if (LEX_STRING_EQUAL(s, "use"))
	return USE;
      else if (LEX_STRING_EQUAL(s, "set"))
	return SET;

      return STRINGID;
    }

  return c;
}

void
yyerror (struct symtab *symtab, char const *s)
{
  notify_script_err_msg (SYNTAX_ERROR, yylloc.first_line, yylloc.last_line, s);
}

void
lexer_init (const char *text)
{
  stream->text = text;
  stream->length = strlen(text);
  stream->index = 0;

  yylloc.first_line = yylloc.last_line = 1;
}

str_ptr
get_lex_string (lex_string_t *lstr)
{
  str_ptr result = str_new ();
  str_copy_c_substr (result, stream->text + lstr->index, lstr->length);
  return result;
}

int
lex_string_equal (lex_string_t *s, const char *ref)
{
  int rlen = strlen (ref);
  if (rlen != s->length)
    return 0;
  return (strncmp (stream->text + s->index, ref, (size_t)rlen) == 0);
}
