/* $Id: find.c,v 1.1 2009-04-29 23:32:01 dpt Exp $ */

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

wimp_i icon_find(wimp_window_info *defn, wimp_icon_flags mask, wimp_icon_flags want)
{
  wimp_icon *p;
  wimp_icon *q;

  for (p = defn->icons, q = p + defn->icon_count; p < q; p++)
    if ((p->flags & mask) == want)
      break;

  if (p < q)
    return (wimp_i) (p - defn->icons);
  else
    return wimp_ICON_WINDOW; /* ie. no icon matched */
}
