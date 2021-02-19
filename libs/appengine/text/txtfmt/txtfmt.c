/* --------------------------------------------------------------------------
 *    Name: txtfmt.c
 * Purpose: Text formatting
 * ----------------------------------------------------------------------- */

/* Text formatting library. Wraps at character widths, not measured widths,
 * so works best for monospaced text.
 *
 * - Breaks at spaces.
 * - Forces a newline at \n.
 *
 * Break input string into spans of (start,length). The list of spans need
 * not cover the whole input string because we'll want to remove newlines
 * from string.
 *
 * When initially created the text is unwrapped.
 */

/* TODO
 *
 * Need to scan the text and set nspans even if it's unwrapped, so the client
 * can tell how many lines it uses.
 */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/base/errors.h"

#include "appengine/text/txtfmt.h"

/* Default height of a line of text */
#define DEFAULT_LINE_HEIGHT 44

/* ----------------------------------------------------------------------- */

typedef struct span
{
  unsigned short start;
  unsigned short length;
}
span;

struct txtfmt_t
{
  char *s;             /* the string we're formatting */
  int   length;        /* string length excluding terminator */

  span *spans;
  int   nspans;
  int   allocated;

  int   width;         /* requested width */

  int   wrapped_width; /* actual minimum width after formatting */

  int   line_height;   /* OS units */
};

enum
{
  StartAt = 1,
  GrowBy  = 2
};

/* ----------------------------------------------------------------------- */

result_t txtfmt_create(const char *s, txtfmt_t **tx)
{
  result_t     err;
  txtfmt_t *newtx;
  size_t    length;

  newtx = calloc(1, sizeof(*newtx));
  if (newtx == NULL)
    goto OOM;

  length = strlen(s);

  newtx->s = malloc(length + 1); /* include terminator */
  if (newtx->s == NULL)
    goto OOM;

  memcpy(newtx->s, s, length + 1);

  newtx->length = length;

  newtx->spans = malloc(StartAt * sizeof(*newtx->spans));
  if (newtx->spans == NULL)
    goto OOM;

  newtx->nspans          = 1;
  newtx->allocated       = StartAt;

  /* initially the string is unwrapped */
  newtx->spans[0].start  = 0;
  newtx->spans[0].length = length;

  newtx->width           = 0;

  newtx->wrapped_width   = length;

  newtx->line_height     = DEFAULT_LINE_HEIGHT;

  *tx = newtx;

  return result_OK;


OOM:

  err = result_OOM;

  txtfmt_destroy(newtx);

  return err;
}

void txtfmt_destroy(txtfmt_t *tx)
{
  if (tx)
  {
    free(tx->spans);
    free(tx->s);
    free(tx);
  }
}

/* ----------------------------------------------------------------------- */

static result_t emit_line(txtfmt_t *tx, int start, int length)
{
  int i;

  i = tx->nspans;

  if (i + 1 == tx->allocated)
  {
    int   n;
    span *newspans;

    /* doubling strategy */

    n = tx->allocated * 2;
    if (n < StartAt)
      n = StartAt;

    newspans = realloc(tx->spans, sizeof(*tx->spans) * n);
    if (newspans == NULL)
      return result_OOM;

    tx->spans     = newspans;
    tx->allocated = n;
  }

  tx->spans[i].start  = start;
  tx->spans[i].length = length;

  if (length > tx->wrapped_width)
    tx->wrapped_width = length;

  tx->nspans = ++i;

  return result_OK;
}

/* wrap to a character width */
result_t txtfmt_wrap(txtfmt_t *tx, int width)
{
  result_t       err;
  const char *startofline;
  int         curlen;
  const char *p;

  if (tx->width == width)
    return result_OK; /* already the required size */

  /* reset the spans */

  tx->nspans        = 0;

  tx->wrapped_width = 0;

  startofline       = tx->s;
  curlen            = 0;

  for (p = tx->s; *p != '\0'; )
  {
    int spacelen;
    int wordlen;

    /* deal with newlines */

    if (*p == '\n' || *p == '\r')
    {
      err = emit_line(tx, startofline - tx->s, curlen);
      if (err)
        return err;

      startofline = NULL;
      p++; /* skip newline character */
      curlen      = 0;

      continue;
    }

    /* count leading spaces */

    spacelen = strspn(p, " ");

    /* quit if no more words are left */

    if (p[spacelen] == '\0')
      break;

    /* get length of word */

    wordlen = strcspn(p + spacelen, " \n\r");

    /* we can't break until we've placed something on the line */

    if (curlen == 0)
    {
      startofline = p + spacelen;
      p          += spacelen + wordlen;
      curlen      = wordlen;

      continue;
    }

    if (curlen + spacelen + wordlen > width)
    {
      /* emit the line */

      err = emit_line(tx, startofline - tx->s, curlen);
      if (err)
        return err;

      startofline = NULL;
      curlen      = 0;

      /* now we jump back and re-measure the spaces and word length */
    }
    else
    {
      /* add the word to the to-be-output counts */

      p      += spacelen + wordlen;
      curlen += spacelen + wordlen;
    }
  }

  /* take care of trailing characters */

  if (curlen)
  {
    err = emit_line(tx, startofline - tx->s, curlen);
    if (err)
      return err;
  }

  return result_OK;
}

/* ----------------------------------------------------------------------- */

static result_t txtfmt_plot_text(const txtfmt_t *tx,
                              const char     *s,
                              int             length,
                              int             x0,
                              int             x1,
                              int             y0,
                              wimp_colour     bgcolour)
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
  icon.extent.y1 = y0 + tx->line_height;
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

result_t txtfmt_paint(const txtfmt_t *tx, int x, int y, wimp_colour bgcolour)
{
  result_t       err;
  int         i;

  assert(tx);
  assert(tx->spans);

  for (i = 0; i < tx->nspans; i++)
  {
    int         start;
    int         length;
    const char *s;

    start  = tx->spans[i].start;
    length = tx->spans[i].length;
    s      = tx->s + start;

    /* we have to plot even if the length is zero, so we can fill in the
     * background */

    err = txtfmt_plot_text(tx, s, length, x, x + 16834, y, bgcolour);
    if (err)
      goto Failure;

    y -= tx->line_height;
  }

  return result_OK;


Failure:

  return err;
}

void txtfmt_set_line_height(txtfmt_t *tx, int line_height)
{
  tx->line_height = line_height;
}

/* ----------------------------------------------------------------------- */

result_t txtfmt_print(const txtfmt_t *tx)
{
  int i;

  assert(tx);
  assert(tx->spans);

  for (i = 0; i < tx->nspans; i++)
  {
    int         start;
    int         length;
    const char *s;

    start  = tx->spans[i].start;
    length = tx->spans[i].length;
    s      = tx->s + start;

    if (length == 0)
      printf("%3d: (%d,%d)\n", i, start, length);
    else
      printf("%3d: %.*s (%d,%d)\n", i, length, s, start, length);
  }

  return result_OK;
}

/* ----------------------------------------------------------------------- */

int txtfmt_get_height(const txtfmt_t *tx)
{
  return tx->nspans;
}

int txtfmt_get_length(const txtfmt_t *tx)
{
  return tx->length;
}

int txtfmt_get_wrapped_width(const txtfmt_t *tx)
{
  return tx->wrapped_width;
}
