
#include "dispers-classes.h"
#include "dispers.h"

struct disp_class_node *disp_class_list; 

struct disp_class *
disp_class_lookup (int tp)
{
  struct disp_class_node *node;

  for (node = disp_class_list; node; node = node->next)
    {
      if (node->value->disp_class_id == tp)
	break;
    }

  if (node == NULL)
    return NULL;

  return node->value;
}

void *
disp_class_next (void *iter)
{
  struct disp_class_node *node = (struct disp_class_node *) iter;

  if (node == NULL)
    return disp_class_list;
  
  return node->next;
}

struct disp_class *
disp_class_from_iter (void *iter)
{
  struct disp_class_node *node = (struct disp_class_node *) iter;
  return (node ? node->value : NULL);
}

struct disp_class_node *
class_list_add_node (struct disp_class *dclass, struct disp_class_node *prev)
{
  struct disp_class_node *node;
  node = emalloc (sizeof(struct disp_class_node));
  node->value = dclass;
  node->next = prev;
  return node;
}

int
init_class_list ()
{
  struct disp_class_node *node = NULL;

  node = class_list_add_node (& bruggeman_disp_class, node);
  node = class_list_add_node (& ho_disp_class, node);
  node = class_list_add_node (& cauchy_disp_class, node);
  node = class_list_add_node (& disp_table_class, node);
  node = class_list_add_node (& disp_sample_table_class, node);
  node = class_list_add_node (& disp_lookup_class, node);

  disp_class_list = node;

  return 0;
}
