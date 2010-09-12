/* $Id: init.c,v 1.2 2008-08-05 22:05:05 dpt Exp $ */

#include "appengine/datastruct/list.h"

void list__init(list_t *anchor)
{
  anchor->next = NULL;
}
