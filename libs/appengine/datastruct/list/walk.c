
#include "appengine/datastruct/list.h"

void list_walk(list_t              *anchor,
                list_walk_callback  cb,
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
