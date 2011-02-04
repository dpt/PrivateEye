/* --------------------------------------------------------------------------
 *    Name: destroy.c
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

void scroll_list__destroy(scroll_list *doomed)
{
  scroll_list__internal_set_handlers(0, doomed->w, doomed);

  wimp_delete_window(doomed->w);

  free(doomed);
}
