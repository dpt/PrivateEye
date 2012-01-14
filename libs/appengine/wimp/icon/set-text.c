
#include <stdio.h>
#include <string.h>

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"
#include "appengine/base/strings.h"

#define BUTTON_TYPE_IS(FLAGS, TYPE) \
  (((FLAGS & wimp_ICON_BUTTON_TYPE) >> wimp_ICON_BUTTON_TYPE_SHIFT) == TYPE)

static void copytrunc(wimp_icon *icon, const char *text, int len)
{
  int              trunclen;
  wimp_icon_flags  alignment;
  const char      *fmt;
  const char      *src;

  /* indirected_text.size includes terminator */

  if (len + 1 <= icon->data.indirected_text.size)
  {
    /* The text will fit: copy the text across including terminator. */
    memcpy(icon->data.indirected_text.text, text, len + 1);
    return;
  }

  /* The text would not fit: try to truncate it. */

  /* Do we have space to add an ellipsis? */
  trunclen = icon->data.indirected_text.size - 1 - 3;
  if (trunclen < 1)
    return; /* not enough space - do nothing */

  alignment = icon->flags & (wimp_ICON_HCENTRED | wimp_ICON_RJUSTIFIED);

  switch (alignment)
  {
  default:
    /* left or centre aligned - truncate on the right */
    fmt = "%.*s...";
    src = text;
    break;

  case wimp_ICON_RJUSTIFIED:
    /* right aligned - truncate on the left */
    fmt = "...%.*s";
    src = text + len - trunclen;
    break;
  }

  sprintf(icon->data.indirected_text.text, fmt, trunclen, src);
}

void icon_set_text(wimp_w w, wimp_i i, const char *text)
{
  wimp_icon_state state;
  int             oldlen;
  int             newlen;

  state.w = w;
  state.i = i;
  wimp_get_icon_state(&state);

  if ((state.icon.flags & (wimp_ICON_TEXT | wimp_ICON_INDIRECTED)) == 0)
    return; /* not text + indirected */

  /* The old text could be terminated with any control character.
   * The new text will always be zero terminated. */

  /* Note that since state.icon.data.indirected_text.text is in the same
   * position for both indirected text and indirected text+sprite icon types,
   * using the former works for both. */

  oldlen = str_len(state.icon.data.indirected_text.text);
  newlen = strlen(text);

  if (oldlen == newlen &&
      memcmp(state.icon.data.indirected_text.text, text, newlen) == 0)
  {
    return; /* exactly the same contents */
  }

  copytrunc(&state.icon, text, newlen);

  /* Update the caret position if it's in the icon. */
  if (BUTTON_TYPE_IS(state.icon.flags, wimp_BUTTON_WRITABLE))
  {
    wimp_caret caret;

    wimp_get_caret_position(&caret);

    if (caret.w == w && caret.i == i)
    {
      int index;

      index = caret.index;
      if (caret.index >= oldlen) /* was it at the end of the old text? */
        index = newlen;

      wimp_set_caret_position(w,
                              i,
                              caret.pos.x,
                              caret.pos.y,
                              caret.height,
                              index);
    }
  }

  /* Force a redraw of the icon. */
  wimp_set_icon_state(w, i, 0, 0);
}
