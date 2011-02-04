
#include "appengine/datastruct/list.h"

void list__add_to_head(list_t *anchor,
                       list_t *item)
{
  item->next = anchor->next;
  anchor->next = item;
}
