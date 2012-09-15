/* --------------------------------------------------------------------------
 *    Name: layout.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "fortify/fortify.h"

#include "oslib/colourtrans.h"
#include "oslib/font.h"
#include "oslib/wimp.h"
#include "oslib/wimpreadsysinfo.h"
#include "oslib/osfile.h"

#include "appengine/types.h"
#include "appengine/base/bitwise.h"
#include "appengine/datastruct/atom.h"
#include "appengine/geom/box.h"
#include "appengine/wimp/event.h"
#include "appengine/vdu/font.h"

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

#define SPACESTRING    " \xB7 " /* small bullet */
#define SPACESTRINGLEN 3

/* ----------------------------------------------------------------------- */

#define sizeof_COLOUR    8
#define sizeof_MOVE_X    4
#define sizeof_MOVE_Y    4
#define sizeof_UNDERLINE 3
#define sizeof_HANDLE    2
#define sizeof_TRANSFORM 20 /* 20 is the worst case. it's (17..20) bytes */

/* We remember our last set_* values so we can skip duplicates. */
static struct set_state
{
  os_colour bg,fg;
  int       pos,thickness;
  font_f    f;
  int       a,b,c,d;
}
set_state;

static void set_reset(void)
{
  memset(&set_state, 0, sizeof(set_state));
}

static char *set_colour(char *s, os_colour bg, os_colour fg)
{
  if (set_state.bg != bg || set_state.fg != fg)
  {
    *s++ = 19;
    *s++ = (bg >>  8) & 0xFF;
    *s++ = (bg >> 16) & 0xFF;
    *s++ = (bg >> 24) & 0xFF;
    *s++ = (fg >>  8) & 0xFF;
    *s++ = (fg >> 16) & 0xFF;
    *s++ = (fg >> 24) & 0xFF;
    *s++ = 14; /* maximum foreground colour offset */
  }

  return s;
}

static char *move_x(char *s, int x)
{
  *s++ = 9;
  *s++ = (x >>  0) & 0xFF;
  *s++ = (x >>  8) & 0xFF;
  *s++ = (x >> 16) & 0xFF;
  return s;
}

static char *move_y(char *s, int y)
{
  *s++ = 11;
  *s++ = (y >>  0) & 0xFF;
  *s++ = (y >>  8) & 0xFF;
  *s++ = (y >> 16) & 0xFF;
  return s;
}

static char *set_underline(char *s, int pos, int thickness)
{
  if (set_state.pos != pos || set_state.thickness != thickness)
  {
    *s++ = 25;
    *s++ = pos;
    *s++ = thickness;
  }

  return s;
}

static char *set_handle(char *s, font_f f)
{
  if (set_state.f != f)
  {
    *s++ = 26;
    *s++ = f;
  }

  return s;
}

static char *set_transform(char *s, int a, int b, int c, int d)
{
  int *p;

  if (set_state.a == a && set_state.b == b &&
      set_state.c == c && set_state.d == d)
    return s; /* no change */

  *s++ = 27;
  while ((int) s & 3)
    *s++ = 0;

  p = (int *) s;

  *p++ = a;
  *p++ = b;
  *p++ = c;
  *p++ = d;

  return (char *) p;
}

/* ----------------------------------------------------------------------- */

static error scales_calc(tag_cloud *tc)
{
  if (tc->display_type == tag_cloud_DISPLAY_TYPE_CLOUD)
  {
    /* del.icio.us uses 80,90,100,115,150% { -4, -2, 0, 3, 10 } */
    /* differences from 100% divided by 5. */
    static const signed char scaletab[] = { 0, 2, 4, 7, 14 };

    int *ps;
    int  i;

    ps = &tc->scaletab[0];
    for (i = 0; i < NELEMS(scaletab); i++)
      *ps++ = (1 << 8) * (100 + tc->config.scale * scaletab[i] * 5 / 100) / 100;
  }

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static error paintstring_new(tag_cloud *tc)
{
  if (tc->layout.paintstring.string == NULL)
  {
    int   n;
    char *string;

    n = tc->e_used * 64; /* initial estimate */
    string = malloc(sizeof(*tc->layout.paintstring.string) * n);
    if (string == NULL)
      return error_OOM;

    tc->layout.paintstring.string    = string;
    tc->layout.paintstring.allocated = n;
  }

  tc->layout.paintstring.used = 0;

  return error_OK;
}

static error paintstring_ensure(tag_cloud *tc, int need, char **ptr)
{
  int   allocated;
  int   used;
  char *string;

  allocated = tc->layout.paintstring.allocated;
  used      = tc->layout.paintstring.used;

  if (allocated - used >= need)
    return error_OK;

  allocated = power2gt(used + need);

  string = realloc(tc->layout.paintstring.string,
                   sizeof(*tc->layout.paintstring.string) * allocated);
  if (string == NULL)
    return error_OOM;

  if (ptr)
    *ptr += string - tc->layout.paintstring.string;

  tc->layout.paintstring.string    = string;
  tc->layout.paintstring.allocated = allocated;

  return error_OK;
}

static void paintstring_del(tag_cloud *tc)
{
  if (tc == NULL)
    return;

  free(tc->layout.paintstring.string);
  tc->layout.paintstring.string    = NULL;
  tc->layout.paintstring.allocated = 0;
  tc->layout.paintstring.used      = 0;
}

/* ----------------------------------------------------------------------- */

static error metrics_new(tag_cloud *tc)
{
  int N;

  N = tc->e_used;

  if (N > tc->layout.dims.allocated)
  {
    tag_cloud_dim *dims;

    dims = realloc(tc->layout.dims.dims, N * sizeof(*dims));
    if (dims == NULL)
      return error_OOM;

    tc->layout.dims.dims      = dims;
    tc->layout.dims.allocated = N;
  }

  return error_OK;
}

static void metrics_del(tag_cloud *tc)
{
  if (tc == NULL)
    return;

  free(tc->layout.dims.dims);
  tc->layout.dims.dims      = NULL;
  tc->layout.dims.allocated = 0;
}

enum
{
  LargeWidth = font_INCH * 10,
};

static error metrics_calc(tag_cloud *tc)
{
  error            err;
  tag_cloud_dims *dims;
  int              i;

  dims = &tc->layout.dims;

  err = metrics_new(tc);
  if (err)
    return err;

  /* work out lengths. determine scales */

  for (i = 0; i < tc->e_used; i++)
  {
    const char *s;
    size_t      len;
    os_trfm     trfm;
    int         scale;

    s = (const char *) atom_get(tc->dict, i, &len);
    assert(s != NULL);

    len--; /* discount terminator */

    scale = (tc->entries[i].count - tc->scale.min) / tc->scale.scale;
    scale = CLAMP(scale, 0, 4);

    dims->dims[i].scale = scale;

    scale = tc->scaletab[scale] << 8;
    trfm.entries[0][0] = scale;
    trfm.entries[0][1] = 0;
    trfm.entries[1][0] = 0;
    trfm.entries[1][1] = scale;
    trfm.entries[2][0] = 0;
    trfm.entries[2][1] = 0;

    font_scan_string(tc->layout.font,
                     s,
                     font_GIVEN_TRFM | font_GIVEN_FONT | font_KERN,
                     LargeWidth, LargeWidth,
                     NULL,
                    &trfm,
                     len,
                     NULL,
                    &dims->dims[i].length[0], NULL,
                     NULL);

    font_scan_string(tc->layout.bold_font,
                     s,
                     font_GIVEN_TRFM | font_GIVEN_FONT | font_KERN,
                     LargeWidth, LargeWidth,
                     NULL,
                    &trfm,
                     len,
                     NULL,
                    &dims->dims[i].length[1], NULL,
                     NULL);
  }

  /* measure the length of the gap string */

  font_scan_string(tc->layout.font,
                   SPACESTRING,
                   font_GIVEN_FONT | font_KERN,
                   LargeWidth, LargeWidth,
                   NULL,
                   NULL,
                   SPACESTRINGLEN,
                   NULL,
                  &dims->gaplength, NULL,
                   NULL);

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static error lengths_new(tag_cloud *tc)
{
  int N;

  N = tc->e_used;

  /* allocate one per tag */
  if (N > tc->layout.lengths.allocated)
  {
    int *length;

    length = realloc(tc->layout.lengths.length, N * sizeof(*length));
    if (length == NULL)
      return error_OOM;

    tc->layout.lengths.length    = length;
    tc->layout.lengths.allocated = N;
  }

  return error_OK;
}

static void lengths_del(tag_cloud *tc)
{
  if (tc == NULL)
    return;

  free(tc->layout.lengths.length);
  tc->layout.lengths.length    = NULL;
  tc->layout.lengths.allocated = 0;
}

/* Build a table of lengths dependent on highlight status. This saves
 * complicating the inner loop.
 */
static error lengths_calc(tag_cloud *tc)
{
  error err;
  int   N;
  int   widest;
  int   i;

  N = tc->e_used;

  err = lengths_new(tc);
  if (err)
    return err;

  widest = 0;

  for (i = 0; i < N; i++)
  {
    int highlight;
    int l;

    highlight = tag_cloud_is_highlighted(tc, i);

    l = tc->layout.dims.dims[i].length[highlight];

    if (l > widest)
      widest = l;

    tc->layout.lengths.length[i] = l;
  }

  tc->layout.lengths.widest = widest;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static error boxes_new(tag_cloud *tc)
{
  int N;

  N = tc->e_used;

  /* allocate one box per tag */
  if (N > tc->layout.boxes.allocated)
  {
    os_box *boxes;

    boxes = realloc(tc->layout.boxes.boxes, N * sizeof(*boxes));
    if (boxes == NULL)
      return error_OOM;

    tc->layout.boxes.boxes     = boxes;
    tc->layout.boxes.allocated = N;
  }

  return error_OK;
}

static void boxes_del(tag_cloud *tc)
{
  if (tc == NULL)
    return;

  free(tc->layout.boxes.boxes);
  tc->layout.boxes.boxes     = NULL;
  tc->layout.boxes.allocated = 0;
}

/* ----------------------------------------------------------------------- */

error tag_cloud_layout_prepare(tag_cloud *tc)
{
  static const char default_font[]      = "Homerton.Medium";
  static const char default_bold_font[] = "Homerton.Bold";

  os_error               *err;
  error                   err2;
  const char             *font_p;
  const char             *bold_font_p;
  int                     font_size;
  font_f                  fh;
  char                    font_id[128];
  char                    bold_font_id[128];
  font_metrics_misc_info  misc_info;
  int                     misc_info_size;

  if (tc->flags & tag_cloud_FLAG_LAYOUT_PREPED)
    return error_OK;

  /* defaults */

  font_p      = default_font;
  bold_font_p = default_bold_font;
  font_size   = 12 * 16;

  /* read the desktop font handle, get its name and use it, if present */

  fh = wimpreadsysinfo_font(NULL);
  if (fh != 0) /* a non-system font */
  {
    char        *p;
    font_attrs  attrs;
    font_attrs  weight;

    /* normal */

    font_read_defn(fh, (byte *) font_id, NULL, NULL, NULL, NULL, NULL, NULL);

    /* zero terminate */
    for (p = font_id; *p >= ' '; p++)
      ;
    *p = '\0';

    font_p = font_id;

    /* find a bold(er) version */

    attrs = font_get_attrs(font_id);
    weight = attrs & font_WEIGHT_MASK;
    weight += font_WEIGHT_BOLD - font_WEIGHT_NORMAL;
    if (weight > font_WEIGHT_950)
      weight = font_WEIGHT_950;
    attrs = (attrs & ~font_WEIGHT_MASK) | weight;
    err2 = font_select(font_id, attrs, bold_font_id, sizeof(bold_font_id));
    if (err2)
      return err2;

    bold_font_p = bold_font_id;

    if (tc->config.size <= 0)
    {
      const char *env;

      /* read the desktop font size, if present */

      env = getenv("Wimp$FontSize");
      if (env)
      {
        int efs;

        efs = atoi(env); /* already multiplied by 16 */
        if (efs)
          font_size = efs;
      }
    }
    else
    {
      font_size = tc->config.size * 16;
    }
  }

  err = xfont_find_font(font_p,
                        font_size,
                        font_size,
                        0,
                        0,
                       &tc->layout.font,
                        NULL,
                        NULL);
  if (err)
    return error_OS;

  err = xfont_find_font(bold_font_p,
                        font_size,
                        font_size,
                        0,
                        0,
                       &tc->layout.bold_font,
                        NULL,
                        NULL);
  if (err)
    return error_OS;

  /* work out the font size and leading in OS units */

  /* assuming 180 OS units per inch */
  tc->layout.font_size = font_size * 180 / 72 / 16;
  tc->layout.leading   = tc->layout.font_size * tc->config.leading / 256;
  tc->layout.padding   = tc->config.padding;

  tc->layout.paintstring.string    = NULL;
  tc->layout.paintstring.allocated = 0;
  tc->layout.paintstring.used      = 0;

  tc->layout.dims.dims             = NULL;
  tc->layout.dims.allocated        = 0;

  font_read_font_metrics(tc->layout.font,
                         NULL,
                         NULL,
                         NULL,
                        &misc_info,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                        &misc_info_size,
                         NULL);

  if (misc_info_size)
  {
    tc->layout.descender = misc_info.descender;
  }
  else
  {
    /* if the font is old then misc_info may not be available: use
     * font_read_info instead, though it's less accurate */

    int y0;

    font_read_info(tc->layout.font, NULL, &y0, NULL, NULL);
    /* returns OS units */

    tc->layout.descender = y0 * font_OS_UNIT;
  }

  /* any hover info is likely to now be invalidated */
  tag_cloud_hover_init(tc);

  tc->flags |= tag_cloud_FLAG_LAYOUT_PREPED;

  return error_OK;
}

void tag_cloud_layout_discard(tag_cloud *tc)
{
  if (tc == NULL)
    return;

  if ((tc->flags & tag_cloud_FLAG_LAYOUT_PREPED) == 0)
    return;

  paintstring_del(tc);
  metrics_del(tc);
  lengths_del(tc);
  boxes_del(tc);

  font_lose_font(tc->layout.bold_font);
  tc->layout.bold_font             = 0;
  font_lose_font(tc->layout.font);
  tc->layout.font                  = 0;

  tc->flags &= ~tag_cloud_FLAG_LAYOUT_PREPED;
}

/* ----------------------------------------------------------------------- */

error tag_cloud_layout(tag_cloud *tc, int width)
{
  error      err = error_OK;
  int        i;
  int        totalwidth;
  os_colour  colours[3];
  char      *p;
  int        x;
  int        linestart;
  int        j;
  int        scaledw;
  int        last_x_move;
  int        last_y_move;
  int        lineheight;
  int        y;

  err = tag_cloud_layout_prepare(tc);
  if (err)
    goto cleanup;

  if ((tc->flags & tag_cloud_FLAG_NEW_ALL) == 0 &&
       tc->layout.width == width)
  {
    return error_OK; /* nothing has changed */
  }

  /* call if: new data, new highlights, new display */
  if (tc->flags & (tag_cloud_FLAG_NEW_DATA       |
                   tag_cloud_FLAG_NEW_HIGHLIGHTS |
                   tag_cloud_FLAG_NEW_DISPLAY))
  {
    tag_cloud_hover_init(tc);
  }

  /* call if: tc->config.scale changes */
  if (tc->flags & tag_cloud_FLAG_NEW_DISPLAY)
  {
    err = scales_calc(tc);
    if (err)
      goto cleanup;
  }

  /* call if: new data, (but not new highlights), new display */
  if (tc->flags & (tag_cloud_FLAG_NEW_DATA       |
                   tag_cloud_FLAG_NEW_DISPLAY))
  {
    err = metrics_calc(tc);
    if (err)
      goto cleanup;
  }

  /* call if: new data, new highlights, new display */
  /* i.e. always in this case */
  err = lengths_calc(tc);
  if (err)
    goto cleanup;

  tc->flags &= ~tag_cloud_FLAG_NEW_ALL;


  err = paintstring_new(tc);
  if (err)
    goto cleanup;


  err = boxes_new(tc);
  if (err)
    goto cleanup;


  colours[0] = os_COLOUR_VERY_LIGHT_GREY;  /* background */
  if (tc->flags & tag_cloud_FLAG_SHADED)
  {
    /* if we're shaded - draw stuff greyed out */

    colours[1] = os_COLOUR_MID_DARK_GREY;  /* regular */
    colours[2] = os_COLOUR_VERY_DARK_GREY; /* highlight */
  }
  else
  {
    colours[1] = 0xff404000u;              /* regular */
    colours[2] = os_COLOUR_BLUE;           /* highlight */
  }


  p = tc->layout.paintstring.string;

  err = paintstring_ensure(tc,
                           sizeof_COLOUR + sizeof_MOVE_X + sizeof_MOVE_Y,
                          &p);
  if (err)
    return err;

  /* reset the set_* state */

  set_reset();

  /* colours */

  p = set_colour(p, colours[0], colours[1]);

  /* padding */

  p = move_x(p,  tc->layout.padding * font_OS_UNIT);
  p = move_y(p, -tc->layout.padding * font_OS_UNIT);

  tc->layout.paintstring.used = p - tc->layout.paintstring.string;


  linestart = 0;

  i = 0; /* index into sorted[] */

  scaledw = (width - 2 * tc->layout.padding) * font_OS_UNIT;

  y = 0;

  while (i < tc->e_used)
  {
    int    index;
    int    remain;
    int    largestscale;
    os_box b;

    /* find out how many tags will fit on this line */

    x = tc->layout.lengths.length[tc->sorted[i]];
    i++; /* let at least one always fit */

    while (i < tc->e_used)
    {
      int x1;

      x1 = x +
           tc->layout.dims.gaplength +
           tc->layout.lengths.length[tc->sorted[i]];
      if (x1 >= scaledw)
        break;

      x = x1;
      i++;
    }

    /* 'i' is the index of the first which didn't fit */

    totalwidth = x;

    remain = scaledw - totalwidth;

    /* allow space for stub moves which we'll insert later */
    err = paintstring_ensure(tc, sizeof_MOVE_X + sizeof_MOVE_Y, &p);
    if (err)
      return err;

    /* remember these values as offsets in case the block is moved */
    last_x_move = p - tc->layout.paintstring.string;
    p += sizeof_MOVE_X;
    last_y_move = p - tc->layout.paintstring.string;
    p += sizeof_MOVE_Y;

    tc->layout.paintstring.used = p - tc->layout.paintstring.string;

    largestscale = 0;

    /* emit a line of tags */

    x = tc->layout.padding * font_OS_UNIT; /* left hand padding */

    for (j = linestart; j < i; j++)
    {
      size_t      l;
      const char *s;
      int         highlight;
      int         hover;
      int         scale;

      index = tc->sorted[j];

      s = (const char *) atom_get(tc->dict, index, &l);

      l--; /* discount terminator */

      /* allocate enough to keep us in business (worst case) */

      err = paintstring_ensure(tc,
                               SPACESTRINGLEN   +
                               sizeof_COLOUR    +
                               sizeof_HANDLE    +
                               sizeof_UNDERLINE +
                               sizeof_TRANSFORM +
                               l                +
                               sizeof_TRANSFORM +
                               sizeof_UNDERLINE +
                               sizeof_HANDLE    +
                               sizeof_COLOUR,
                              &p);
      if (err)
        return err;

      /* if not the first token on the line insert a space */

      if (j > linestart)
      {
        memcpy(p, SPACESTRING, SPACESTRINGLEN);
        p += SPACESTRINGLEN;
        x += tc->layout.dims.gaplength;
      }

      highlight = tag_cloud_is_highlighted(tc, index);
      if (highlight)
      {
        p = set_colour(p, colours[0], colours[2]);
        p = set_handle(p, tc->layout.bold_font);
      }

      hover = (index == tc->hover.index);
      if (hover)
        p = set_underline(p, -32, 24);

      scale = tc->layout.dims.dims[index].scale;
      if (scale > largestscale)
        largestscale = scale;
      scale = tc->scaletab[scale] << 8;
      p = set_transform(p, scale, 0, 0, scale);

      /* emit the tag text */

      memcpy(p, s, l);
      p += l;

      /* restore */

      p = set_transform(p, 1 << 16, 0, 0, 1 << 16);

      if (hover)
        p = set_underline(p, -32, 0);

      if (highlight)
      {
        p = set_handle(p, tc->layout.font);
        p = set_colour(p, colours[0], colours[1]);
      }

      tc->layout.paintstring.used = p - tc->layout.paintstring.string;

      /* remember where the tag was placed */

      b.x0 = x;
      b.x1 = x + tc->layout.lengths.length[index];
      b.y0 = (tc->layout.descender >> 8) * scale / 65536;
      b.y1 = tc->layout.font_size * scale / 65536;

      tc->layout.boxes.boxes[index] = b;

      x = b.x1; /* x += length[index]; */
    }

    /* set the line position */

    /* horizontally centre the line */

    (void) move_x(tc->layout.paintstring.string + last_x_move, remain / 2);

    /* vertically space the line */

    lineheight = tc->layout.leading * tc->scaletab[largestscale] / 256;
    (void) move_y(tc->layout.paintstring.string + last_y_move,
                  -lineheight * font_OS_UNIT);
    y -= lineheight;

    /* if not the last line, reset the line horizontally */

    if (i < tc->e_used)
    {
      err = paintstring_ensure(tc, sizeof_MOVE_X, &p);
      if (err)
        return err;

      p = move_x(p, -(remain / 2 + totalwidth));

      tc->layout.paintstring.used = p - tc->layout.paintstring.string;
    }

    /* now we know where all the boxes lie on this line, we can adjust their
     * positions */

    for (j = linestart; j < i; j++)
    {
      index = tc->sorted[j];

      b = tc->layout.boxes.boxes[index];

      /* these need scaling into OS units */
      b.x0 = (b.x0 + remain / 2) / font_OS_UNIT;
      b.x1 = (b.x1 + remain / 2) / font_OS_UNIT;

      b.y0 += y - tc->layout.padding;
      b.y1 += y - tc->layout.padding;

      tc->layout.boxes.boxes[index] = b;

      /* round the boxes to whole pixels */
      box_round4(&tc->layout.boxes.boxes[index]);
    }

    linestart = i;
  }

#ifndef NDEBUG
  fprintf(stderr, "paintstring.used = %d\n", tc->layout.paintstring.used);
#endif

  tc->layout.width  = width;
  tc->layout.height = -y + 2 * tc->layout.padding; /* OS units */

cleanup:

  return err;
}

/* ----------------------------------------------------------------------- */

int tag_cloud_hit(tag_cloud *tc, int x, int y)
{
  int i;

  for (i = 0; i < tc->e_used; i++)
    if (box_contains_point(&tc->layout.boxes.boxes[i], x, y))
      break;

  return (i == tc->e_used) ? -1 : i;
}
