/* $Id: set-title.c,v 1.2 2009-05-04 22:29:38 dpt Exp $ */

#include <string.h>

#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

void window_set_title_text(wimp_w w, const char *text)
{
  wimp_window_info  info;
  char             *title;
  int               size;

  info.w = w;
  wimp_get_window_info_header_only(&info);

  if ((info.title_flags & wimp_ICON_TEXT) == 0)
    return;

  if (info.title_flags & wimp_ICON_INDIRECTED)
  {
    /* indirected text */
    title = info.title_data.indirected_text.text;
    size  = info.title_data.indirected_text.size - 1;
  }
  else
  {
    /* text */
    title = info.title_data.text;
    size  = 11;
  }

  strncpy(title, text, size);

  if (info.flags & wimp_WINDOW_OPEN)
    wimp_force_redraw_furniture(w, wimp_FURNITURE_TITLE);
}
