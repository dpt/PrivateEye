/* --------------------------------------------------------------------------
 *    Name: sprite-data.h
 * Purpose: Sprite data processing
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_SPRITE_H
#define APPENGINE_SPRITE_H

#include <stddef.h>

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/osspriteop.h"

#include "appengine/base/errors.h"

/* ----------------------------------------------------------------------- */

/* Supplement until this stuff arrives in OSLib */

#define osspriteop_EXT_STYLE                   ((osspriteop_mode_word) 0x78000001u) /* includes osspriteop_NEW_STYLE */
#define osspriteop_EXT_STYLE_MASK              ((osspriteop_mode_word) 0x780F000Fu)
#define osspriteop_EXT_XRES                    ((osspriteop_mode_word) 0x00000030u)
#define osspriteop_EXT_XRES_SHIFT              (4)
#define osspriteop_EXT_YRES                    ((osspriteop_mode_word) 0x000000C0u)
#define osspriteop_EXT_YRES_SHIFT              (6)
#define osspriteop_EXT_MODE_FLAGS              ((osspriteop_mode_word) 0x0000FF00u)
#define osspriteop_EXT_MODE_FLAGS_SHIFT        (8)
#define osspriteop_EXT_TYPE                    ((osspriteop_mode_word) 0x07F00000u)
#define osspriteop_EXT_TYPE_SHIFT              (20)

#define osspriteop_EXT_RES_180                 (0)
#define osspriteop_EXT_RES_90                  (1)
#define osspriteop_EXT_RES_45                  (2)
#define osspriteop_EXT_RES_23                  (3)

#define osspriteop_TYPE_24BPP                  ((osspriteop_mode_word) 0x08u) /* RISC OS 3.5/6 defined this but no-one properly supports it */
#define osspriteop_TYPE_JPEG                   ((osspriteop_mode_word) 0x09u) /* RISC OS 3.5/6 defined this but no-one properly supports it */ 
#define osspriteop_TYPE_EXT                    ((osspriteop_mode_word) 0x0Fu) /* RISC OS 5.21+ */
#define osspriteop_TYPE_16BPP4K                ((osspriteop_mode_word) 0x10u) /* RISC OS 5.21+ */
#define osspriteop_TYPE_420YCC                 ((osspriteop_mode_word) 0x11u) /* RISC OS 5.21+ */
#define osspriteop_TYPE_422YCC                 ((osspriteop_mode_word) 0x12u) /* RISC OS 5.21+ */

// os_MODE_FLAG_DATA_FORMAT_RGB is in OSLib
// os_MODE_FLAG_DATA_FORMAT_CMYK is in OSLib but ought to be supplemented with MISC:
#define os_MODE_FLAG_DATA_FORMAT_MISC          ((os_mode_flags) 0x1u) /* RISC OS 5.21+ */
#define os_MODE_FLAG_DATA_FORMAT_YCBCR         ((os_mode_flags) 0x2u) /* RISC OS 5.21+ */

#define os_MODE_FLAG_DATA_SUBFORMAT_SHIFT      (14)

#define os_MODE_FLAG_DATA_SUBFORMAT_TBGR       ((os_mode_flags) 0x0u) // technically this is on 4.32+
#define os_MODE_FLAG_DATA_SUBFORMAT_TRGB       ((os_mode_flags) 0x1u) /* RISC OS 5.21+ */
#define os_MODE_FLAG_DATA_SUBFORMAT_ABGR       ((os_mode_flags) 0x2u) /* RISC OS 5.21+ */
#define os_MODE_FLAG_DATA_SUBFORMAT_ARGB       ((os_mode_flags) 0x3u) /* RISC OS 5.21+ */

#define os_MODE_FLAG_DATA_SUBFORMAT_KYMC       ((os_mode_flags) 0x0u) // technically this is on 4.32+

#define os_MODE_FLAG_DATA_SUBFORMAT_YCC601FULL ((os_mode_flags) 0x0u) /* RISC OS 5.21+ */
#define os_MODE_FLAG_DATA_SUBFORMAT_YCC601VID  ((os_mode_flags) 0x1u) /* RISC OS 5.21+ */
#define os_MODE_FLAG_DATA_SUBFORMAT_YCC709FULL ((os_mode_flags) 0x2u) /* RISC OS 5.21+ */
#define os_MODE_FLAG_DATA_SUBFORMAT_YCC709VID  ((os_mode_flags) 0x3u) /* RISC OS 5.21+ */

/* ----------------------------------------------------------------------- */

/* Returns the N'th sprite in the area */
osspriteop_header *sprite_select(const osspriteop_area *area, int n);

/* Calculates the byte size of a sprite (sprite header and data) */
size_t sprite_bytes(int w, int h, int log2bpp, osbool paletted);

/* Returns the byte size of a sprite (sprite data only) */
size_t sprite_data_bytes(const osspriteop_header *header);

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

int sprite_type(osspriteop_mode_word mode_word);

/* Returns whether a sprite (really) has alpha channel data */
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
result_t sprite_get_histograms(osspriteop_area   *area,
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

result_t sprite_remap(osspriteop_area   *area,
                      osspriteop_header *src,
                      osspriteop_header *dst,
                      sprite_luts       *luts);

result_t sprite_remap_luma(osspriteop_area   *area,
                           osspriteop_header *src,
                           osspriteop_header *dst,
                           sprite_lut        *lut);

/* ----------------------------------------------------------------------- */

const os_palette *get_default_palette(int log2bpp);

int sprite_colours(osspriteop_area      **area_anc,
                   osspriteop_header     *header,
                   osspriteop_trans_tab **trans_tab_anc);

/* Returns the best mode for the specified args.
 * If set, inline_alpha will attempt to find an ABGR mode etc. 
 * Returns -1 if failure. */
os_mode sprite_mode(int xeig, int yeig, int log2bpp, osbool inline_alpha);

/* Return a string describing the specified mode. */
result_t sprite_describe_mode(os_mode mode, char *desc, size_t sz);

/* Adds a mask covering the specified pixel value */
void sprite_mask_pixel(osspriteop_area   *area,
                       osspriteop_header *header,
                       unsigned int       value);

#endif /* APPENGINE_SPRITE_H */
