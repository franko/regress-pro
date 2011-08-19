#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "str-util.h"
#include <ctype.h>

int
str_getcwd (str_t dir)
{
  size_t bsize;

  for (bsize = 64; /* */; bsize *= 2)
    {
      str_size_check  (dir, bsize);
      if (getcwd (dir->heap, bsize) == dir->heap)
	{
	  dir->length = strlen (dir->heap);
	  return 0;
	}
      if (errno != ERANGE)
	break;
    }

  return 1;
}

int
str_loadfile (const char *filename, str_t text)
{
  struct stat info[1];
  FILE *fp;
  int j, n, tsize;

  if(stat (filename, info) != 0)
    return 1;

  tsize = info->st_size;

  fp = fopen (filename, "r");
  if (fp == NULL)
    return 1;

  str_size_check (text, tsize + 1);

  n = fread (text->heap, 1, tsize, fp);
#ifndef WIN32
  if(n != tsize)
    {
      str_trunc (text, 0);
      fclose (fp);
      return 1;
    }
#endif

  text->length = n;
  text->heap[n] = '\0';

  /* This is a dirty hack to get rid of the stupid \015 on Windows */
  for (j = 0; j < tsize; j++)
    if (text->heap[j] == '\015')
      text->heap[j] = ' ';

  fclose (fp);

  return 0;
}

int
str_is_abs_pathname (str_t path)
{
#ifdef WIN32
  int ch, j = 0;
  if (j >= path->length)
    return 0;

  ch = path->heap[0];
  j++;

  if (! isalpha (ch) || j >= path->length)
    return 0;

  if (! strstr (path->heap, ":\\"))
    return 0;
  
  return 1;
#else
  if (path->length == 0)
    return 0;
  return (path->heap[0] == '/');
#endif
}
