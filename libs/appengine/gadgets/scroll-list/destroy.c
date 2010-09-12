/* --------------------------------------------------------------------------
 *    Name: destroy.c
 * Purpose: Scrolling list
 * Version: $Id: destroy.c,v 1.2 2008-07-27 18:59:04 dpt Exp $
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
