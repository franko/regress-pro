/*
  $Id: gui-test.c,v 1.1 2006/12/26 18:16:08 francesco Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "common.h"
#include "str.h"
#include "symtab.h"
#include "fit-engine.h"
#include "error-messages.h"
#include "grid-search.h"
#include "multi-fit-engine.h"
#include "lmfit-multi.h"
#include "error-messages.h"

enum token {
  LOAD = 0,
  EDIT,
  SPECTRUM,
  SCRIPT,
  RUN,
  FIT,
  BATCH,
  QUIT,
  MULTIFIT,
  V_STRING = 255
};

struct tvalue {
  enum token token;
  union {
    str_ptr string;
    int num;
  } val;
};

static void on_cmd_load_spectrum (str_ptr s);
static void on_cmd_load_script (str_ptr s);
static void on_cmd_run_fit (void);
static void on_cmd_run_multi_fit (void);

static int loadfile(const char *filename, str_ptr text);
static int  mylexer (struct tvalue *v);
static int  process_guitext_events (void *data, float prog, const char *msg);
static void set_fit_strategy (str_ptr text);
static void print_fit_error_msg (void);

extern int main (void);

static const char *keyword[] = {
  "load", "edit", "spectrum", "script", "run", "fit", "batch", "quit",
  "multifit"
};
static const int keywords_nb = 9;

struct {
  str_t script;
  str_t error_msg;

  struct spectrum *spectr;

  struct symtab symtab[1];
} app[1];

static void
char_append (str_t s, int c)
{
  char cc[2] = {c, 0};
  str_append_c (s, cc, 0);
}

static int
take_word (str_t word)
{
  int c, quoted = 0;

  str_set_null (word);

  do
    c = fgetc (stdin);
  while (c == ' ' || c == '\n' || c == EOF);

  if (c == EOF)
    return EOF;

  if (c == '"')
    quoted = 1;
  else
    char_append (word, c);

  if (quoted)
    {
      while (1)
	{
	  c = fgetc (stdin);
	  if (c == '"' || c == EOF)
	    break;
	  if (c != '"')
	    char_append (word, c);
	}
    }
  else
    {
      while (1)
	{
	  c = fgetc (stdin);
	  if (c == ' ' || c == '\n' || c == EOF)
	    break;
	  char_append (word, c);
	}
    }

  return 0;
}

int
mylexer (struct tvalue *v)
{
  str_ptr word;
  int status, j;

  word = emalloc (sizeof(str_t));
  str_init (word, 64);
  status = take_word (word);

  if (status)
    {
      STR_PTR_FREE(word);
      return status;
    }

  for (j = 0; j < keywords_nb; j++)
    if (strcmp (CSTR(word), keyword[j]) == 0)
      break;

  if (j < keywords_nb)
    v->token = j;
  else
    {
      v->token = V_STRING;
      v->val.string = word;
      return 0;
    }

  STR_PTR_FREE(word);
  return 0;
}

#define MYLEX(v) { \
  int status_ = mylexer(v); \
  if (status_) \
    break; \
}

int
main ()
{
  struct tvalue val[1];
  int cycle;

  app->spectr = NULL;

  symbol_table_init (app->symtab);

  str_init (app->script, 128);
  str_init (app->error_msg, 128);

  for (cycle = 1; cycle; )
    {
      printf ("> ");
      fflush (stdout);

      MYLEX(val);
      if (val->token == LOAD)
	{
	  MYLEX(val);
	  if (val->token == SPECTRUM)
	    {
	      MYLEX(val);
	      if (val->token == V_STRING)
		{
		  on_cmd_load_spectrum (val->val.string);
		  goto cycle_cont;
		}
	    }
	  else if (val->token == SCRIPT)
	    {
	      MYLEX(val);
	      if (val->token == V_STRING)
		{
		  on_cmd_load_script (val->val.string);
		  goto cycle_cont;
		}
	    }
	}
      else if (val->token == RUN)
	{
	  MYLEX(val);
	  if (val->token == FIT)
	    {
	      on_cmd_run_fit ();
	      goto cycle_cont;
	    }
	  else if (val->token == MULTIFIT)
	    {
	      on_cmd_run_multi_fit ();
	      goto cycle_cont;
	    }
	}
      else if (val->token == QUIT)
	{
	  cycle = 0;
	  goto cycle_cont;
	}
      else if (val->token == EDIT)
	{
	  str_t line, buffer;
	  str_init (line, 64);
	  str_init (buffer, 64 * 16);
	  printf ("Insert the modified script followed by __END__.\n");
	  for (;;)
	    {
	      str_getline (line, stdin);
	      if (strcmp (CSTR(line), "__END__") == 0)
		break;
	      str_append (buffer, line, '\n');
	    }
	  set_fit_strategy (buffer);
	  str_free (line);
	  str_free (buffer);
	  goto cycle_cont;
	}

      printf ("Syntax error.\n");
    cycle_cont:
      if (val->token == V_STRING)
	STR_PTR_FREE (val->val.string);
    }

  clean_error_msgs ();

  symbol_table_clean (app->symtab);
  symbol_table_free (app->symtab);

  if (app->spectr)
    spectra_free (app->spectr);

  str_free (app->error_msg);
  str_free (app->script);

  return 0;
}

void
on_cmd_load_spectrum (str_ptr s)
{
  if (app->spectr)
    spectra_free (app->spectr);

  app->spectr = load_gener_spectrum (CSTR(s));

  if (app->spectr == NULL)
    printf ("Cannot load spectra %s", CSTR(s));
}

int
loadfile(const char *filename, str_ptr text)
{
  struct stat info[1];
  FILE *fp;
  int j, n;

  if(stat (filename, info) != 0)
    return 1;

  fp = fopen(filename, "r");
  if (fp == NULL)
    return 1;

  str_size_check (text, info->st_size+1);

  n = fread (text->heap, 1, (size_t)info->st_size, fp);

  text->heap[n < info->st_size ? n : info->st_size] = '\0';
  for (j = 0; j < info->st_size; j++)
    if (text->heap[j] == '\015')
      text->heap[j] = ' ';

  fclose (fp);

  return 0;
}

void
print_fit_error_msg ()
{
  get_errors_list (app->error_msg);
  printf ("The parsing of the script has been"
	  " unsuccessful :\n%s", CSTR(app->error_msg));
}

void
on_cmd_load_script (str_ptr s)
{
  if (loadfile (CSTR(s), app->script))
    {
      str_trunc (app->script, 0);
      printf ("Cannot load script file: %s\n", CSTR(s));
      return;
    }

  if (parse_strategy (app->symtab, CSTR(app->script)) != 0)
    {
      print_fit_error_msg ();
      clean_error_msgs ();
    }
}

void
on_cmd_run_multi_fit ()
{
  struct multi_fit_engine *fit;
  gsl_vector *fit_results;
  str_t analysis;
  double chisq;
  int skip_question = 0;
  struct {
    struct seeds *common;
    struct seeds *individual;
  } seeds;

  fit = build_multi_fit_engine (app->symtab,
				&seeds.common, &seeds.individual);

  str_init (analysis, 512);

  lmfit_multi (fit, seeds.common, seeds.individual,
	       &chisq, analysis,
	       app->error_msg, LMFIT_GET_RESULTING_STACK,
	       process_guitext_events, & skip_question);

  // multi_fit_engine_print_fit_results (fit, fp_results);

  str_free (analysis);

  multi_fit_engine_disable (fit);
  multi_fit_engine_free (fit);
}

void
on_cmd_run_fit ()
{
  struct fit_engine *fit;
  struct seeds *seeds;
  gsl_vector *fit_results;
  struct spectrum *gensp;
  int skip_question = 0;
  double chisq;
  str_t analysis;
  str_t fit_error;
  str_t fpresults;

  if (app->spectr == NULL)
    return;

  fit = build_fit_engine (app->symtab, &seeds);

  printf ("Fit is running, Please wait...\n");

  str_init (analysis, 512);

  fit_engine_prepare (fit, app->spectr);

  str_init (fit_error, 8);

  lmfit_grid (fit, seeds, &chisq, analysis, fit_error,
	      LMFIT_GET_RESULTING_STACK,
	      process_guitext_events, & skip_question);

  if (STR_LENGTH(fit_error) != 0)
    {
      printf ("Fit error: %s\n", CSTR(fit_error));
      clean_error_msgs ();
    }
  str_free (fit_error);

  str_init (fpresults, 4096);
  fit_engine_print_fit_results (fit, fpresults, 0);
  printf ("%s\n", CSTR(fpresults));
  str_free (fpresults);

  printf ("Residual Chisq/pt: %f\n", chisq);

  gsl_vector_free (fit_results);

  printf("\n%s", CSTR(analysis));
  str_free (analysis);

  fit_engine_restore_spectr (fit);

  gensp = generate_spectrum (fit);
  spectra_free (gensp);

  fit_engine_disable (fit);

  fit_engine_free (fit);
}

int
process_guitext_events (void *data, float prog, const char *msg)
{
  int *skip = (int *) data;
  char ans[64];

  if (*skip)
    return 0;

  if (msg)
    printf ("%s, ", msg);
  printf ("Progress: %.0f %%\n", prog * 100.0);
  printf ("Stop? ");
  fgets (ans, 64, stdin);

  if (strcmp (ans, "yes\n") == 0)
    return 1;
  else if (strcmp (ans, "continue\n") == 0)
    *skip = 1;

  return 0;
}

void
set_fit_strategy (str_ptr text)
{
  str_copy (app->script, text);
}
