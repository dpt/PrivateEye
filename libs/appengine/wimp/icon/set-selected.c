/* $Id: set-selected.c,v 1.1 2009-04-29 23:32:01 dpt Exp $ */

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

void icon_set_selected(wimp_w w, wimp_i i, int select)
{
  icon_set_flags(w, i, select ? wimp_ICON_SELECTED : 0, wimp_ICON_SELECTED);
}
