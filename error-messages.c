#include "error-messages.h"
#include "common.h"

struct line_info {
  int first, last;
};

struct msg_cell {
  str_t msg;
  struct line_info *line;
  struct msg_cell *next;
};

static struct msg_cell *top = NULL;

static const char * contest_id[] = {
  "Syntax error:",
  "Script error:",
  "While loading file:",
  "Invalid strategy:",
  "Error running fit:"
};

void
notify_error_msg (enum err_contest idxc, const char * msg, ...)
{
  struct msg_cell *cell = emalloc (sizeof(struct msg_cell));
  str_t tail;
  va_list ap;

  str_init_from_c (cell->msg, contest_id[idxc]);
  
  va_start (ap, msg);
  str_init (tail, 64);
  str_vprintf (tail, msg, 0, ap);
  str_append (cell->msg, tail, ' ');
  str_free (tail);
  va_end (ap);

  cell->line = NULL;
  cell->next = top;
  top = cell;
}

void
notify_script_err_msg (enum err_contest idxc, int fline, int lline,
		       const char * msg, ...)
{
  struct msg_cell *cell = emalloc (sizeof(struct msg_cell));
  str_t tail;
  va_list ap;

  str_init_from_c (cell->msg, contest_id[idxc]);
  
  va_start (ap, msg);
  str_init (tail, 64);
  str_vprintf (tail, msg, 0, ap);
  str_append (cell->msg, tail, ' ');
  str_free (tail);
  va_end (ap);

  cell->line = emalloc(sizeof(struct line_info));
  cell->line->first = fline;
  cell->line->last  = lline;
  cell->next = top;
  top = cell;
}

static void
msgs_rec_get (struct msg_cell *c, str_t text)
{
  if (c == NULL)
    return;
  msgs_rec_get (c->next, text);
  str_append (text, c->msg, 0);
  str_append_c (text, "\n", 0);
}

void
get_errors_list (str_t text)
{
  msgs_rec_get (top, text);
}

void
clean_error_msgs (void)
{
  while (top)
    {
      struct msg_cell *tnext = top->next;
      if (top->line)
	free (top->line);
      str_free (top->msg);
      free (top);
      top = tnext;
    }
}

int
get_script_error_region (int *fline, int *lline)
{
  struct msg_cell *cell;

  for (cell = top; cell; cell = cell->next)
    {
      if (cell->line)
	{
	  *fline = cell->line->first;
	  *lline = cell->line->last;
	  return 0;
	}
    }

  return 1;
}
