/* $Id: add-head.c,v 1.2 2008-08-05 22:05:05 dpt Exp $ */

#include "appengine/datastruct/list.h"

void list__add_to_head(list_t *anchor,
                       list_t *item)
{
  item->next = anchor->next;
  anchor->next = item;
}
