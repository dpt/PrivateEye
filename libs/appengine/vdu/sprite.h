/* --------------------------------------------------------------------------
 *    Name: sprite-data.h
 * Purpose: Sprite data processing
 * Version: $Id: sprite.h,v 1.1 2009-05-21 22:27:20 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_SPRITE_H
#define APPENGINE_SPRITE_H

#include "oslib/os.h"
#include "oslib/osspriteop.h"

#include "appengine/base/errors.h"

/* Returns the N'th sprite in the area */
osspriteop_header *sprite_select(const osspriteop_area *area, int n);

/* Calculates the byte size of a sprite */
int sprite_size(int w, int h, int log2bpp);

/* Returns a pointer to the sprite data */
void *sprite_data(const osspriteop_header *header);

/* Returns a pointer to the sprite mask data */
void *sprite_mask_data(const osspriteop_header *header);

/* Retrieves sprite dimensions, mode and log2bpp */
void sprite_info(const osspriteop_area   *area,
                 const osspriteop_header *header,
                 int                     *width,
                 int                     *height,
                 osbool                  *mask,
                 os_mode                 *mode,
                 int                     *log2bpp);

/* Returns whether a sprite has alpha data */
osbool sprite_has_alpha(const osspriteop_header *header);

/* Populates a grey palette */
void make_grey_palette(int log2bpp, unsigned int *palette);

/* ----------------------------------------------------------------------- */

/* A single histogram. */
typedef struct
{
  unsigned int v[256];
}
sprite_histogram;

/* A collection of histograms. */
typedef struct
{
  sprite_histogram h[5]; /* luma, red, green, blue, alpha */
}
sprite_histograms;

/* Calculates Luma, R, G and B histograms. 256 entries each. */
error sprite_get_histograms(osspriteop_area   *area,
                            osspriteop_header *header,
                            sprite_histograms *hists);

/* ----------------------------------------------------------------------- */

/* A single lookup table. */
typedef struct
{
  unsigned char v[256];
}
sprite_lut;

/* A collection of lookup tables. */
typedef struct
{
  sprite_lut l[3];
}
sprite_luts;

error sprite_remap(osspriteop_area   *area,
                   osspriteop_header *src,
                   osspriteop_header *dst,
                   sprite_luts       *luts);

error sprite_remap_luma(osspriteop_area   *area,
                        osspriteop_header *src,
                        osspriteop_header *dst,
                        sprite_lut        *lut);

/* ----------------------------------------------------------------------- */

const os_palette *get_default_palette(int log2bpp);

int sprite_colours(osspriteop_area      **area_anc,
                   osspriteop_header     *header,
                   osspriteop_trans_tab **trans_tab_anc);

/* Returns the best mode for the specified args */
os_mode sprite_mode(int xeig, int yeig, int log2bpp);

/* Adds a mask covering the specified pixel value */
void sprite_mask_pixel(osspriteop_area   *area,
                       osspriteop_header *header,
                       unsigned int       value);

#endif /* APPENGINE_SPRITE_H */
