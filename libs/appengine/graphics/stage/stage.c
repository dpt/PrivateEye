/* --------------------------------------------------------------------------
 *    Name: stage.c
 * Purpose: Generates a document background
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>

#include "appengine/types.h"

#include "appengine/graphics/stage.h"

typedef enum
{
  C00, C10, C20, C30, C40, C50, C60, C70, C80, C90
}
column_t;

typedef enum
{
  R00, R10, R20, R30, R40, R50, R60, R70, R80, R90
}
row_t;

typedef struct stageboxspec
{
  /* This must match the order in config.colours. */
  enum
  {
    stageboxkind_PASTEBOARD,
    stageboxkind_STROKE,
    stageboxkind_MARGIN,
    stageboxkind_IMAGE,
    stageboxkind_SHADOW,
    stageboxkind__LIMIT
  }
  kind;
  column_t x0;
  row_t    y0;
  column_t x1;
  row_t    y1;
}
stageboxspec_t;

static int stagebox_compare(const void *va, const void *vb)
{
  const stagebox_t *a = va;
  const stagebox_t *b = vb;
  int               delta;

  delta = (a->box.y0 - b->box.y0);
  if (delta == 0)
    delta = (a->box.x0 - b->box.x0);

  return (delta < 0) ? -1 : (delta > 0) ? 1 : 0; // correct
}

/* Sum the input sizes (widths/heights) and output an array of positions.
 * Note that the output is zero prefixed, so is one longer than the input,
 * e.g. 10,20,10 becomes 0,10,30,40. */
static void makepos(const int *sizes, size_t nsizes, int *pos)
{
  int t;

  t = 0;
  while (nsizes--)
  {
    *pos++ = t;
    t += *sizes++;
  }
  *pos++ = t;
}

/* Resolve arrays of stageboxspecs into a list of stageboxes. Input specs are
 * in any order. Output boxes are sorted by their top left position. */
static void resolve_specs(const stageboxspec_t *speclists[],
                                size_t          lenlist[],
                                size_t          nspecs,
                          const int            *xsizes,
                                size_t          nxsizes,
                          const int            *ysizes,
                                size_t          nysizes,
                          const os_colour      *colours,
                                stagebox_t     *boxes,
                                size_t         *nboxes)
{
  int         xpos[nxsizes + 1], ypos[nysizes + 1];
  size_t      i,j;
  stagebox_t *boxp;

  assert(speclists);
  assert(lenlist);
  assert(nspecs > 0);
  assert(xsizes);
  assert(nxsizes > 0);
  assert(ysizes);
  assert(nysizes > 0);
  assert(colours);
  assert(boxes);
  assert(nboxes);

  /* Populate position arrays */
  makepos(xsizes, nxsizes, xpos);
  makepos(ysizes, nysizes, ypos);

  /* Resolve the lists of box specs to a single list of boxes */
  boxp = boxes;
  for (j = 0; j < nspecs; j++)
  {
    const stageboxspec_t *spec = speclists[j];
    size_t                len  = lenlist[j];

    for (i = 0; i < len; i++)
    {
      boxp->colour = colours[spec->kind];
      boxp->box.x0 = xpos[spec->x0];
      boxp->box.y0 = ypos[spec->y0];
      boxp->box.x1 = xpos[spec->x1];
      boxp->box.y1 = ypos[spec->y1];
      spec++;
      boxp++;
    }
  }

  *nboxes = boxp - boxes;

  /* Sort laid out boxes by their top left coordinate */
  qsort(boxes, *nboxes, sizeof(*boxes), stagebox_compare);
}

void stage_generate(const stageconfig_t *config,
                    int                  workarea_width,
                    int                  workarea_height,
                    int                  page_width,
                    int                  page_height,
                    int                 *min_workarea_width,
                    int                 *min_workarea_height,
                    stagebox_t          *boxes,
                    size_t              *nboxes)
{
  static const stageboxspec_t plain_page[] =
  {
    { stageboxkind_PASTEBOARD, C00, R80, C90, R90 }, // top
    { stageboxkind_PASTEBOARD, C00, R10, C10, R80 }, // left
    { stageboxkind_PASTEBOARD, C70, R10, C90, R80 }, // right
    { stageboxkind_PASTEBOARD, C00, R00, C90, R20 }, // bottom
  };

  static const stageboxspec_t shadowed_page[] =
  {
    { stageboxkind_PASTEBOARD, C00, R80, C90, R90 }, // top
    { stageboxkind_PASTEBOARD, C00, R10, C10, R80 }, // left
    { stageboxkind_PASTEBOARD, C80, R10, C90, R80 }, // right
    { stageboxkind_PASTEBOARD, C00, R00, C90, R10 }, // bottom
    { stageboxkind_PASTEBOARD, C10, R10, C30, R20 }, // left bottom shim
    { stageboxkind_PASTEBOARD, C70, R60, C80, R80 }, // right top shim

    { stageboxkind_SHADOW,     C70, R10, C80, R60 }, // right
    { stageboxkind_SHADOW,     C30, R10, C70, R20 }, // bottom
  };

  static const stageboxspec_t common[] =
  {
    { stageboxkind_STROKE,     C10, R70, C70, R80 }, // top
    { stageboxkind_STROKE,     C10, R20, C20, R70 }, // left
    { stageboxkind_STROKE,     C60, R20, C70, R70 }, // right
    { stageboxkind_STROKE,     C10, R20, C70, R30 }, // bottom

    { stageboxkind_MARGIN,     C20, R50, C60, R70 }, // top
    { stageboxkind_MARGIN,     C20, R40, C40, R50 }, // left
    { stageboxkind_MARGIN,     C50, R40, C60, R50 }, // right
    { stageboxkind_MARGIN,     C20, R30, C60, R40 }, // bottom
  };

  static const stageboxspec_t page_only[] =
  {
    { stageboxkind_IMAGE,      C40, R40, C50, R50 }, // image
  };

  const stageboxspec_t  *specs[3];
  size_t                 nspecs[3];
  const stageboxspec_t **pspecs;
  size_t                *pnspecs;
  int                    x_sizes[9], y_sizes[9]; /* Careful Now */
  int                   *p;
  int                    xeig, yeig;
  int                    i;

  assert(config);
  assert(boxes);
  assert(nboxes);

  pspecs  = &specs[0];
  pnspecs = &nspecs[0];
  if (config->flags & stage_FLAG_SHADOW)
  {
    *pspecs++  = shadowed_page;
    *pnspecs++ = NELEMS(shadowed_page);
  }
  else
  {
    *pspecs++  = plain_page;
    *pnspecs++ = NELEMS(plain_page);
  }
  *pspecs++  = common;
  *pnspecs++ = NELEMS(common);
  if (config->flags & stage_FLAG_PAGE)
  {
    *pspecs++  = page_only;
    *pnspecs++ = NELEMS(page_only);
  }

  typedef enum dims
  {
    StrokeW,
    StrokeH,
    MarginW,
    MarginH,
    PageW,
    PageH,
    ShadowW,
    ShadowH,
    dims__LIMIT
  }
  dims_t;

  dims_t d[dims__LIMIT] = /* d = dimensions */
  {
    config->stroke, /* width */
    config->stroke, /* height */
    config->margin,
    config->margin,
    page_width,
    page_height,
    config->shadow,
    config->shadow,
  };

  os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_XEIG_FACTOR, &xeig);
  os_read_mode_variable(os_CURRENT_MODE, os_MODEVAR_YEIG_FACTOR, &yeig);
  xeig = (1 << xeig) - 1;
  yeig = (1 << yeig) - 1;
#define ROUNDEDX(x) (((x) + xeig) & ~xeig)
#define ROUNDEDY(y) (((y) + yeig) & ~yeig)

  /* Round the dimensions to match the screen mode EIG factors. */
  for (i = 0; i < dims__LIMIT; i++)
    d[i] = ((i & 1) == 0) ? ROUNDEDX(d[i]) : ROUNDEDY(d[i]);

  /* The stroked border outline is considered to be atop the margin so we can
   * ignore it here. The shadow is considered be part of the variable
   * pasteboard area.
   */
  int x_fixed    = d[MarginW] + d[PageW] + d[MarginW];
  int y_fixed    = d[MarginH] + d[PageH] + d[MarginH];
  int x_variable = workarea_width  - x_fixed;
  int y_variable = workarea_height - y_fixed;

  x_variable = MAX(x_variable, config->pasteboard_min);
  y_variable = MAX(y_variable, config->pasteboard_min);
  if (min_workarea_width)
    *min_workarea_width  = x_variable + x_fixed;
  if (min_workarea_height)
    *min_workarea_height = y_variable + y_fixed;

  /* Build the grid */

  /* Columns */
  p = x_sizes;
  *p++ = ROUNDEDX(x_variable / 2); // left - pasteboard
  *p++ = d[StrokeW];
  *p++ = d[ShadowW] - d[StrokeW];
  *p++ = d[MarginW] - d[ShadowW]; // stroke widths cancel out
  *p++ = d[PageW];
  *p++ = d[MarginW] - d[StrokeW];
  *p++ = d[StrokeW];
  *p++ = d[ShadowW];
  *p++ = x_variable - ROUNDEDX(x_variable / 2) - d[ShadowW]; // right hand side takes up any slack

  /* Rows */
  p = y_sizes;
  *p++ = ROUNDEDY(y_variable / 2) - d[ShadowH]; // bottom - pasteboard
  *p++ = d[ShadowH];
  *p++ = d[StrokeH];
  *p++ = d[MarginH] - d[StrokeH];
  *p++ = d[PageH];
  *p++ = d[ShadowH]; // top of shadow
  *p++ = d[MarginH] - d[ShadowH] - d[StrokeH];
  *p++ = d[StrokeH];
  *p++ = y_variable - ROUNDEDY(y_variable / 2); // top edge takes takes up any slack

  resolve_specs(specs, nspecs, pnspecs - &nspecs[0],
                x_sizes, NELEMS(x_sizes),
                y_sizes, NELEMS(y_sizes),
(const os_colour *) &config->colours, /* treat as an array */
                boxes,
                nboxes);
}
