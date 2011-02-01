/* --------------------------------------------------------------------------
 *    Name: drawable-jpeg.c
 * Purpose: Stuff for drawing JPEGs, distinct from loading
 * Version: $Id: drawable-jpeg.c,v 1.4 2009-09-13 17:02:13 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "kernel.h"
#include "swis.h"

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/jpeg.h"
#include "oslib/os.h"
#include "oslib/draw.h"
#include "oslib/osfile.h"
#include "oslib/colourtrans.h"
#include "oslib/wimp.h"

#include "appengine/vdu/screen.h"
#include "appengine/base/oserror.h"

#include "drawable-bitmap.h"
#include "drawable-jpeg.h"

static void jpeg_redraw(const drawable_choices *choices, wimp_draw *draw, drawable_t *drawable, int x, int y)
{
  os_error         *e;
  const jpeg_image *jpeg;

  NOT_USED(draw);

  jpeg = drawable->image->image;

  e = EC(xjpeg_plot_scaled(jpeg,
                           x, y,
                          &drawable->details.jpeg.factors,
                           drawable->image->display.file_size,
                           choices->jpeg.plot_flags));
  if (e)
    oserror__plot(e, x, y);
}

// almost identical to bitmap_scaling
static void jpeg_scaling(drawable_t *drawable, const os_factors *factors)
{
  int         image_xeig, image_yeig;
  int         screen_xeig, screen_yeig;
  os_factors *scaled_factors;

  image_xeig = drawable->image->display.dims.bm.xeig;
  image_yeig = drawable->image->display.dims.bm.yeig;

  read_current_mode_vars(&screen_xeig, &screen_yeig, NULL);

  scaled_factors = &drawable->details.jpeg.factors;
  scaled_factors->xmul = factors->xmul << image_xeig;
  scaled_factors->ymul = factors->ymul << image_yeig;
  scaled_factors->xdiv = factors->xdiv << screen_xeig;
  scaled_factors->ydiv = factors->ydiv << screen_yeig;
}

void drawablejpeg_export_methods(drawable_t *drawable)
{
  static const drawable_methods methods =
  {
    jpeg_redraw,
    NULL,
    jpeg_scaling,
    bitmap_get_dimensions,
    NULL,
    NULL
  };

  drawable->methods = methods;
}
