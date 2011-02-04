/* --------------------------------------------------------------------------
 *    Name: bitmap.c
 * Purpose: Generic bitmap viewer methods
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "flex.h"

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/colourtrans.h"
#include "oslib/draw.h"
#include "oslib/hourglass.h"
#include "oslib/osfile.h"
#include "oslib/wimp.h"

#include "appengine/base/oserror.h"
#include "appengine/base/messages.h"
#include "appengine/vdu/screen.h"
#include "appengine/vdu/sprite.h"
#include "appengine/geom/trfm.h"

#include "bitmap.h"

int bitmap_save(image_choices *choices, image_t *image, const char *file_name)
{
  osspriteop_area *area;
  os_error        *e;

  NOT_USED(choices);

  area = (osspriteop_area *) image->image;
  e = EC(xosspriteop_save_sprite_file(osspriteop_PTR, area, file_name));
  if (e)
  {
    oserror__report_block(e);
    return TRUE; /* failure */
  }

  image->flags &= ~image_FLAG_MODIFIED;

  return FALSE; /* success */
}

int bitmap_unload(image_t *image)
{
  if (image != NULL)
    if (image->image != NULL)
      flex_free(&image->image);

  return FALSE; /* success */
}

int bitmap_histogram(image_t *image)
{
  osspriteop_area   *area;
  osspriteop_header *header;
  sprite_histograms *hists;

  area   = (osspriteop_area *) image->image;
  header = sprite_select(area, 0);
  hists  = image->hists;

  sprite_get_histograms(area, header, hists);

  return FALSE; /* success */
}

/* preserve palettes
 * cope with non-square modes
 * test thoroughly
 */

/* 1,3,5,7: hard rotation */
static int rotate_hard(image_t *image, int angle)
{
  osspriteop_area      *area;
  osspriteop_header    *header;
  osspriteop_area      *newarea;
  osspriteop_header    *newheader;
  int                   w,h;
  int                   neww,newh;
  size_t                sprimgbytes;
  size_t                sprrowbytes;
  int                   c0,c1,c2,c3;
  os_trfm               t;
  osspriteop_trans_tab *trans_tab;
  int                   log2bpp;
  int                   xeig,yeig;

  area   = (osspriteop_area *) image->image;
  header = sprite_select(area, 0);

  w = image->display.dims.bm.width;
  h = image->display.dims.bm.height;

  /* work out sprite area size */

  read_mode_vars(header->mode, &xeig, &yeig, &log2bpp);

  neww = (h << yeig) >> xeig;
  newh = (w << xeig) >> yeig;

  sprrowbytes = (((neww << log2bpp) + 31) & ~31) >> 3;
  sprimgbytes = sizeof(osspriteop_area) + sizeof(osspriteop_header);
  sprimgbytes += sprrowbytes * newh;

  /* alloc area */

  if (flex_alloc((flex_ptr) &newarea, sprimgbytes) == 0)
    goto NoMem;

  /* HEAP HAS MOVED! - update pointers */
  area   = (osspriteop_area *) image->image;
  header = sprite_select(area, 0);

  /* init area */

  newarea->size  = sprimgbytes;
  newarea->first = 16;
  osspriteop_clear_sprites(osspriteop_USER_AREA, newarea);

  /* create sprite */
  /* this should be identical to the original, save for dimensions */

  /* ### masks won't work yet */

  osspriteop_create_sprite(osspriteop_NAME,
                           newarea,
                           header->name,
                           0, /* no palette */ /* ### */
                           neww, newh,
                           header->mode);

  /* redirect to sprite */

  newheader = sprite_select(newarea, 0);

  osspriteop_switch_output_to_sprite(osspriteop_PTR,
                                     newarea,
                     (osspriteop_id) newheader,
                                     0,
                                     &c0, &c1, &c2, &c3);

  /* set up drawing stuff */

  if (sprite_colours((osspriteop_area **) &image->image, header, &trans_tab))
    goto NoMem;

  /* problem: we're switched to a sprite which _may_ have moved now
   * so as a work-around, unswitch output and update pointers, then switch
   * back */

  /* redirect back */

  osspriteop_unswitch_output(c0, c1, c2, c3);

  /* HEAP HAS MOVED! - update pointers */
  area   = (osspriteop_area *) image->image;
  header = sprite_select(area, 0);

  /* redirect to sprite */

  newheader = sprite_select(newarea, 0);

  osspriteop_switch_output_to_sprite(osspriteop_PTR,
                                     newarea,
                     (osspriteop_id) newheader,
                                     0,
                                     &c0, &c1, &c2, &c3);

  /* dimensions need to be in OS units now */

  w <<= xeig;
  h <<= yeig;

  neww <<= xeig; /* these might be the wrong way around */
  newh <<= yeig;

  /* set up our transform */

  trfm_set_identity(&t);

  /* flip horizontally */
  if (angle >= 4)
  {
    t.entries[0][0] = -t.entries[0][0];
    t.entries[2][0] += w;
  }

  trfm_translate(&t, -w / 2, -h / 2);

  trfm_rotate_degs(&t, (angle & 3) * 90 * 65536);

  trfm_translate(&t, neww / 2, newh / 2);

  t.entries[0][0] *= 256;
  t.entries[0][1] *= 256;
  t.entries[1][0] *= 256;
  t.entries[1][1] *= 256;
  t.entries[2][0] *= 256;
  t.entries[2][1] *= 256;

  /* draw */

  osspriteop_put_sprite_trfm(osspriteop_PTR,
                             area,
             (osspriteop_id) header,
                             0, /* trfm_flags */
                             NULL, /* source_rect */
                             osspriteop_USE_MASK, /* action */
                            &t,
                             trans_tab);

  /* redirect back */

  osspriteop_unswitch_output(c0, c1, c2, c3);

  if (trans_tab)
    flex_free((flex_ptr) &trans_tab);

  /* replace previous sprite */

  flex_free((flex_ptr) &image->image);

  image->display.dims.bm.width  = neww >> xeig; /* see note above */
  image->display.dims.bm.height = newh >> yeig;

  flex_reanchor((flex_ptr) &image->image, (flex_ptr) &newarea);
  image->display.file_size = sprimgbytes;

  return FALSE; /* success */


NoMem:

  oserror__report(0, "error.no.mem");
  return TRUE; /* failure */
}

/* 0,2,4,6: easy flips */
static int rotate_easy(image_t *image, int angle)
{
  enum { FlipH = 1, FlipV = 2 };

  osspriteop_area   *area;
  osspriteop_header *header;

  switch (angle)
  {
  case 2:  angle = FlipH | FlipV; break; /* 2 -> 3 */
  case 4:  angle = FlipH;         break; /* 4 -> 1 */
  case 6:  angle = FlipV;         break; /* 6 -> 2 */
  default: assert(0);             return FALSE; /* success */
  }

  area   = (osspriteop_area *) image->image;
  header = sprite_select(area, 0);

  if (angle & FlipH) /* flip horz */
  {
    osspriteop_flip_about_yaxis(osspriteop_PTR,
                                area,
                (osspriteop_id) header);
  }

  if (angle & FlipV) /* flip vert */
  {
    osspriteop_flip_about_xaxis(osspriteop_PTR,
                                area,
                (osspriteop_id) header);
  }

  return FALSE; /* success */
}

int bitmap_rotate(image_choices *choices, image_t *image, int angle)
{
  int r;

  NOT_USED(choices);

  hourglass_on();

  image_about_to_modify(image);

  angle /= 90; /* 0..719 -> 0..7 */

  if (angle & 1)
    r = rotate_hard(image, angle);
  else
    r = rotate_easy(image, angle);

  image_modified(image, image_MODIFIED_DATA);

  hourglass_off();

  return r;
}
