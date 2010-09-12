/* $Id: get-defn.c,v 1.1 2009-04-29 23:32:01 dpt Exp $ */

#include <stddef.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

wimp_window_info *window_get_defn(wimp_w w)
{
  wimp_window_info  header;
  size_t            size;
  wimp_window_info *defn;

  header.w = w;
  wimp_get_window_info_header_only(&header);

  size = wimp_SIZEOF_WINDOW_INFO(header.icon_count);

  defn = malloc(size);
  if (defn == NULL)
    return defn;

  defn->w = w;
  wimp_get_window_info(defn);

  return defn;
}
