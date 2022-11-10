/* --------------------------------------------------------------------------
 *    Name: generic-bitmap.c
 * Purpose: Generic bitmap methods
 * ----------------------------------------------------------------------- */

#include "kernel.h"
#include "swis.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fortify/fortify.h"

#include "flex.h"

#include "appengine/base/oserror.h"
#include "appengine/base/messages.h"
#include "appengine/vdu/screen.h"
#include "appengine/vdu/sprite.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/draw.h"
#include "oslib/osfile.h"
#include "oslib/osspriteop.h"
#include "oslib/wimp.h"

#include "appengine/graphics/drawable.h"
#include "appengine/graphics/image.h"

#include "drawable-bitmap.h"

/* ----------------------------------------------------------------------- */

#define Tinct_PlotAlpha         (0x57240)
#define Tinct_PlotScaledAlpha   (0x57241)
#define Tinct_Plot              (0x57242)
#define Tinct_PlotScaled        (0x57243)
#define Tinct_ConvertSprite     (0x57244)
#define Tinct_AvailableFeatures (0x57245)
#define Tinct_Compress          (0x57246)
#define Tinct_Decompress        (0x57247)

#define tinct_DITHER            (1u<<2)
#define tinct_DIFFUSE           (1u<<3)

#define tinct_SPRITE_TYPE       (0x301680B5) /* 90x90dpi, 24bpp */

static void redraw_tinct(const drawable_choices *choices,
                         wimp_draw              *draw,
                         drawable_t             *drawable,
                         int                     x,
                         int                     y)
{
  const os_error    *e;
  osspriteop_area   *area;
  osspriteop_header *header;
  int                w,h;
  unsigned int       flags;
  int                scale;
  int                swi;

  NOT_USED(draw);

  assert(drawable->image->refcount > 0);

  area   = (osspriteop_area *) drawable->image->image;
  header = sprite_select(area, drawable->image->details.sprite.index);

  /* Tinct simplifies the job of scaling by cutting out the need to pass in
   * scaling factors. This means all we really need here is the current
   * scale. But since that is a property of the view we don't have it handy.
   * Since we've been passed it earlier, work it out from the scale factors.
   */
#define SCALE_100PC 100
  scale = drawable->details.sprite.factors.xmul * SCALE_100PC /
          drawable->details.sprite.factors.xdiv;

  w = drawable->image->display.dims.bm.width  * 2 * scale / SCALE_100PC;
  h = drawable->image->display.dims.bm.height * 2 * scale / SCALE_100PC;

  flags = (choices->sprite.plot_flags == osspriteop_DITHERED) ? tinct_DIFFUSE : 0;

  swi = (drawable->image->flags & image_FLAG_HAS_ALPHA) ?
        Tinct_PlotScaledAlpha : Tinct_PlotScaled;
  e = (const os_error *) _swix(swi, _INR(2,7), header, x, y, w, h, flags);
  if (e)
    oserror_plot(e, x, y);
}

static void redraw_os(const drawable_choices *choices,
                      wimp_draw              *draw,
                      drawable_t             *drawable,
                      int                     x,
                      int                     y)
{
  const os_error       *e;
  osspriteop_area      *area;
  osspriteop_header    *header;
  osspriteop_trans_tab *trans_tab;

  NOT_USED(draw);

  assert(drawable->image->refcount > 0);

  area   = (osspriteop_area *) drawable->image->image;
  header = sprite_select(area, drawable->image->details.sprite.index);

  trans_tab = drawable->details.sprite.trans_tab;

  e = EC(xosspriteop_put_sprite_scaled(osspriteop_PTR,
                                       area,
                       (osspriteop_id) header,
                                       x, y,
                                       choices->sprite.plot_flags |
                                       osspriteop_USE_MASK        |
                                       osspriteop_GIVEN_WIDE_ENTRIES,
                                      &drawable->details.sprite.factors,
                                       trans_tab));
  if (e)
    oserror_plot(e, x, y);
}

typedef void (redraw_fn)(const drawable_choices *choices,
                         wimp_draw              *draw,
                         drawable_t             *drawable,
                         int                     x,
                         int                     y);

static void bitmap_redraw(const drawable_choices *choices,
                          wimp_draw              *draw,
                          drawable_t             *drawable,
                          int                     x,
                          int                     y)
{
  osbool     has_mask;
  os_mode    mode;
  redraw_fn *redraw;

  has_mask = (drawable->image->flags & image_FLAG_HAS_MASK) != 0;
  mode = drawable->image->details.sprite.mode;

  redraw = redraw_os;

  if (choices->sprite.use_tinct != 0 &&
      mode == (os_mode) tinct_SPRITE_TYPE &&
      !has_mask)
  {
    int              features;
    _kernel_oserror *err;

    /* see if Tinct is loaded */
    features = 0;
    err = _swix(Tinct_AvailableFeatures,
                _IN(0)|_OUT(0),
                features,
               &features);
    if (err == NULL)
      redraw = redraw_tinct;
  }

  drawable->methods.redraw = redraw;

  redraw(choices, draw, drawable, x, y);
}

/* ----------------------------------------------------------------------- */

static void bitmap_colours(drawable_t *drawable)
{
  osspriteop_area   *area;
  osspriteop_header *header;

  assert(drawable->image->refcount > 0);

  if (drawable->details.sprite.trans_tab)
    /* Discard previous translation table */
    flex_free((flex_ptr) &drawable->details.sprite.trans_tab);

  area   = drawable->image->image;
  header = sprite_select(area, 0);

  sprite_colours((osspriteop_area **) &drawable->image->image,
                  header,
                 &drawable->details.sprite.trans_tab);
}

static void bitmap_scaling(drawable_t *drawable, const os_factors *factors)
{
  int         image_xeig, image_yeig;
  int         screen_xeig, screen_yeig;
  os_factors *scaled_factors;

  assert(drawable->image->refcount > 0);

  image_xeig = drawable->image->display.dims.bm.xeig;
  image_yeig = drawable->image->display.dims.bm.yeig;

  read_current_mode_vars(&screen_xeig, &screen_yeig, NULL);

  scaled_factors = &drawable->details.sprite.factors;
  scaled_factors->xmul = factors->xmul << image_xeig;
  scaled_factors->ymul = factors->ymul << image_yeig;
  scaled_factors->xdiv = factors->xdiv << screen_xeig;
  scaled_factors->ydiv = factors->ydiv << screen_yeig;
}

void bitmap_get_dimensions(drawable_t       *drawable,
                           const os_factors *factors,
                           os_box           *box)
{
  image_t *image;

  assert(drawable->image->refcount > 0);

  image = drawable->image;

  box->x0 = 0;
  box->y0 = 0;
  box->x1 = (image->display.dims.bm.width  << image->display.dims.bm.xeig) * factors->xmul / factors->xdiv;
  box->y1 = (image->display.dims.bm.height << image->display.dims.bm.yeig) * factors->ymul / factors->ydiv;
}

static void bitmap_reset(drawable_t *drawable)
{
  if (drawable->details.sprite.trans_tab)
    /* Discard previous translation table */
    flex_free((flex_ptr) &drawable->details.sprite.trans_tab);

  drawable->details.sprite.index = 0;
}

static void bitmap_set_index(drawable_t *drawable, int index)
{
  drawable->details.sprite.index = index;
}

void drawablebitmap_export_methods(drawable_t *drawable)
{
  static const drawable_methods methods =
  {
    bitmap_redraw,
    bitmap_colours,
    bitmap_scaling,
    bitmap_get_dimensions,
    bitmap_reset,
    bitmap_set_index
  };

  drawable->details.sprite.trans_tab = NULL;
  drawable->details.sprite.index     = 0;

  drawable->methods = methods;

  if (drawable->image->flags & (image_FLAG_HAS_MASK | image_FLAG_HAS_ALPHA))
    drawable->flags |= drawable_FLAG_DRAW_BG;
}
