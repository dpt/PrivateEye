/* $Id: walk.c,v 1.2 2008-08-05 22:05:05 dpt Exp $ */

#include "appengine/datastruct/list.h"

void list__walk(list_t              *anchor,
                list__walk_callback  cb,
                void                *cbarg)
{
  list_t *e;
  list_t *next;

  /* Be careful walking the list. The callback may be destroying the objects
   * we're handling to it.
   */

  for (e = anchor->next; e != NULL; e = next)
  {
    next = e->next;
    if (cb(e, cbarg) < 0)
      break;
  }
}
