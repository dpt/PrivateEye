/* $Id: get-name.c,v 1.1 2009-04-29 23:32:01 dpt Exp $ */

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

/* Returns the string that follows an N validation string. */
const char *icon_get_name(wimp_w w, wimp_i i)
{
  wimp_icon_state state;
  static char     buffer[12]; /* Careful Now */
  char           *validation, *p, b, c;

  if (i == wimp_ICON_WINDOW)
    return ""; /* not an icon */

  state.w = w;
  state.i = i;
  wimp_get_icon_state(&state);

  if ((state.icon.flags & wimp_ICON_INDIRECTED) == 0)
    return ""; /* not indirected */

  validation = state.icon.data.indirected_text.validation;

  if (validation == NULL || validation == (char *) -1)
    return ""; /* no validation string */

  c = ';';
  do
  {
    b = c;
    c = *validation++;
    if (c < ' ')
      return ""; /* name not found */
  }
  while (b != ';' || (c != 'N' && c != 'n'));

  p = buffer;
  c = *validation++;
  while (c >= ' ' && c != ';')
  {
    *p++ = c;
    c = *validation++;
  }
  *p = '\0';

  return buffer;
}
