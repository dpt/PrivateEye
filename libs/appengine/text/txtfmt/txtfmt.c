/* --------------------------------------------------------------------------
 *    Name: txtfmt.c
 * Purpose: Text formatting
 * ----------------------------------------------------------------------- */

/* Word-wrapping lives in DPTLib's text/txtfmt. This file just paints
 * the resulting lines using the desktop font. */

#include <assert.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "text/txtfmt.h"

#include "appengine/base/errors.h"

#include "appengine/text/txtfmt.h"

/* ----------------------------------------------------------------------- */

static result_t txtfmt_plot_text(const char *s,
                                 int         length,
                                 int         x0,
                                 int         x1,
                                 int         y0,
                                 int         line_height,
                                 wimp_colour bgcolour)
{
  char      buf[length + 1];
  wimp_icon icon;

  /* we have to copy the text into its own buffer because the input text is
   * not terminated */

  memcpy(buf, s, length);
  buf[length] = '\0';

  icon.extent.x0 = x0;
  icon.extent.y0 = y0;
  icon.extent.x1 = x1;
  icon.extent.y1 = y0 + line_height;
  icon.flags = wimp_ICON_TEXT | wimp_ICON_VCENTRED | wimp_ICON_INDIRECTED |
              (wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT);

  if (bgcolour != wimp_COLOUR_TRANSPARENT)
  {
    icon.flags |= wimp_ICON_FILLED |
                 (bgcolour << wimp_ICON_BG_COLOUR_SHIFT);
  }

  icon.data.indirected_text.text       = buf;
  icon.data.indirected_text.validation = "";
  icon.data.indirected_text.size       = length;

  wimp_plot_icon(&icon);

  return result_OK;
}

result_t txtfmt_paint(const txtfmt_t *tx,
                      int             x,
                      int             y,
                      int             line_height,
                      wimp_colour     bgcolour)
{
  result_t err;
  int      nlines;
  int      i;

  assert(tx);

  nlines = txtfmt_get_nlines(tx);

  for (i = 0; i < nlines; i++)
  {
    const char *s;
    int         length;

    err = txtfmt_get_line(tx, i, &s, &length);
    if (err)
      return err;

    /* we have to plot even if the length is zero, so we can fill in the
     * background */

    err = txtfmt_plot_text(s, length, x, x + 16834, y, line_height, bgcolour);
    if (err)
      return err;

    y -= line_height;
  }

  return result_OK;
}
