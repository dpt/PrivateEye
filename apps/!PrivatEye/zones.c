/* --------------------------------------------------------------------------
 *    Name: zones.c
 * Purpose: Zone management
 * Version: $Id: zones.c,v 1.23 2009-11-29 23:18:37 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifdef EYE_ZONES

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/osspriteop.h"
#include "oslib/wimp.h"

#include "appengine/vdu/sprite.h"
#include "appengine/graphics/image.h"

#include "privateeye.h"
#include "viewer.h"
#include "zones.h"

enum { log2zone = 6 }; /* 64x64 zones */

/* Converts pixels to zones. */
#define TOZONE_DOWN(x) (x) >> log2zone
/* Converts pixels to zones, with rounding up. */
#define TOZONE_UP(x) ((x) + ((1 << log2zone) - 1)) >> log2zone

zones *zones_create(image_t *image)
{
  int    w, h;
  int    rowwords;
  size_t size;
  zones *zones;

  /* FIXME: Fix the vector handling case. */
  if (image->flags & image_FLAG_VECTOR)
    return NULL;

  /* The number of zones we need for this image. */
  w = TOZONE_UP(image->display.dims.bm.width);
  h = TOZONE_UP(image->display.dims.bm.height);

  /* FIXME: Width and height isn't the same as the window extent. Width and
   * height might only be valid for bitmap formats anyway. */

  rowwords = (w + 31) >> 5; /* Words in a row (add padding). */
  size = rowwords * 4 * h;  /* Size bytes. */

/* fprintf(stderr,"zones_create: w,h=%d,%d size=%d\n",w,h,size); */

  zones = malloc(size); /* FIXME: Use the sliding heap instead. */
  if (zones == NULL)
    return NULL;

  memset(zones, 0, size);

  return zones;
}

void zones_destroy(zones *zones)
{
  if (zones == NULL)
      return;

  free(zones);
}

void zones_update(zones *zones, wimp_draw *redraw, image_t *image, int scale)
{
  int           w, h;
  int           rowwords;
  int           x0, y0, x1, y1;
  unsigned int *zonerow;
  int           x, y;

  if (zones == NULL)
    return;

  w = TOZONE_UP(image->display.dims.bm.width);
  h = TOZONE_UP(image->display.dims.bm.height);

  rowwords = (w + 31) >> 5; /* Words in a row (add padding). */

  x0 = redraw->xscroll;
  y0 = redraw->yscroll - (redraw->box.y1 - redraw->box.y0);
  x1 = redraw->xscroll + (redraw->box.x1 - redraw->box.x0);
  y1 = redraw->yscroll;

  /* Scale OS units to zoom level. */

  if (scale != SCALE_100PC)
  {
    x0 = x0 * SCALE_100PC / scale;
    y0 = y0 * SCALE_100PC / scale;
    x1 = x1 * SCALE_100PC / scale;
    y1 = y1 * SCALE_100PC / scale;
  }

  /* Shifting right by the eig values scales OS units to pixels. */

  x0 = x0 >> image->display.dims.bm.xeig;
  y0 = y0 >> image->display.dims.bm.yeig;
  x1 = x1 >> image->display.dims.bm.xeig;
  y1 = y1 >> image->display.dims.bm.yeig;

  /* Scale into zone grid elements. */

  x0 = TOZONE_DOWN(x0);
  y0 = TOZONE_DOWN(y0);
  x1 = TOZONE_UP(x1);
  y1 = TOZONE_UP(y1);

/* fprintf(stderr,"x0=%d,y0=%d,x1=%d,y1=%d,row=%d,w,h=%d,%d\n",x0,y0,x1,y1,rowwords,w,h); */

  /* Clamp. */

  if (x0 < 0) { x0 = 0; /*fprintf(stderr, "clamped x0\n");*/ }
  if (y0 < 0) { y0 = 0; /*fprintf(stderr, "clamped y0\n");*/ }
  if (x1 > w) { x1 = w; /*fprintf(stderr, "clamped x1\n");*/ }
  if (y1 > h) { y1 = h; /*fprintf(stderr, "clamped y1\n");*/ }

  /* Invert Y. */

  zonerow = zones + ((h - 1) - y0) * rowwords;
  for (y = y0; y < y1; y++)
  {
    for (x = x0; x < x1; x++)
    {
      zonerow[x >> 5] |= 1u << (x & 31);
    }
    zonerow -= rowwords;
  }

  if (0) /* debug sprite dump code */
  {
    osspriteop_area *area;
    osspriteop_header *sptr;
    static const unsigned int transtab[2] = { 0x00ffffff, 0x00000000 };
    unsigned char *sdat;
    static const os_factors scale = { 16, 16, 2, 2 };
    enum { X = 48, Y = 48 };
    int plot;

/* fprintf(stderr,"w,h=%d,%d\n",w,h); */

    /* a 4000x4000 zonemask with 64-square elements would be ~ 489 bytes. */

    area = malloc(5000);
    if (area == NULL)
      return;

    area->size  = 5000;
    area->first = 16;
    osspriteop_clear_sprites(osspriteop_PTR, area);

    osspriteop_create_sprite(osspriteop_USER_AREA,
                             area,
                             "zones",
                             0,
                             w, h,
                             os_MODE1BPP90X90);

    sptr = sprite_select(area, 0);
    sdat = (unsigned char *) sptr + 44;

    memcpy(sdat, zones, rowwords * 4 * h);

    x0 = redraw->box.x0 - redraw->xscroll + X;
    y0 = redraw->box.y1 - redraw->yscroll + Y;

    /* the transtab setup here will only work properly in 32bpp modes */
    osspriteop_put_sprite_scaled(osspriteop_PTR,
                                 area,
                 (osspriteop_id) sptr,
                                 x0, y0,
                                 os_ACTION_OVERWRITE | osspriteop_GIVEN_WIDE_ENTRIES,
                                &scale,
        (osspriteop_trans_tab *) transtab);

    /* draw a grid */
    /* the scaling is probably wrong */

    plot = 0;
    for (y = 0; y < h; y++)
    {
      int yy = y0 + y * 2 * scale.ymul / scale.ydiv;

      os_plot(4, x0, yy);
      os_plot(6 + plot, x0 + w * 2 * scale.xmul / scale.xdiv, yy);
      plot = 32;
    }

    plot = 0;
    for (x = 0; x < w; x++)
    {
      int xx = x0 + x * 2 * scale.xmul / scale.xdiv;

      os_plot(4, xx, y0);
      os_plot(6 + plot, xx, y0 + h * 2 * scale.ymul / scale.ydiv);
      plot = 32;
    }

    free(area);
  }
}

void zones_wherenext(const zones *zones)
{
  NOT_USED(zones);

  /* NYI: Look for the topmost,rightmost unviewed area? */
}

#else

extern int dummy;

#endif /* EYE_ZONES */
