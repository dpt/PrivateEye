/* --------------------------------------------------------------------------
 *    Name: jpeg.c
 * Purpose: JPEG loader
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "flex.h"

#include "oslib/types.h"
#include "oslib/colourtrans.h"
#include "oslib/draw.h"
#include "oslib/hourglass.h"
#include "oslib/jpeg.h"
#include "oslib/os.h"
#include "oslib/osfile.h"
#include "oslib/wimp.h"

#include "datastruct/ntree.h"

#include "appengine/base/messages.h"
#include "appengine/base/oserror.h"
#include "appengine/base/utils.h"
#include "appengine/graphics/jpegtran.h"
#include "appengine/vdu/sprite.h"

#include "generic.h"
#include "bitmap.h"

#include "jpeg.h"

static int jpeg_to_spr_common(image_t *image);

/* ----------------------------------------------------------------------- */

static result_t jpeg_populate_info(image_t *image, const jpeg_info_t *info)
{
  result_t       err;
  char           buf[100];
  const char    *tok;
  unsigned char  bpc = 8;
  unsigned char  ncomps;

  buf[0] = '\0';

  /* the start of the string is the overall format
   * JPEGs can be JFIF, Exif or Adobe format, or a combination thereof */
  if (info->flags & jpeg_FLAG_JFIF)  { strcat(buf, "JFIF"); }
  if (info->flags & jpeg_FLAG_EXIF)  { if (buf[0] != '\0') strcat(buf, "+"); strcat(buf, "Exif"); }
  if (info->flags & jpeg_FLAG_ADOBE) { if (buf[0] != '\0') strcat(buf, "+"); strcat(buf, "Adobe"); }
 
  /* append a string for the DCT ordering */
  switch (info->flags & (jpeg_FLAG_BASELINE | jpeg_FLAG_EXTSEQ | jpeg_FLAG_PRGRSSVE))
  {
  case jpeg_FLAG_BASELINE: tok = "jpginfo.baseline"; break;
  case jpeg_FLAG_EXTSEQ:   tok = "jpginfo.extseq"; break;
  case jpeg_FLAG_PRGRSSVE: tok = "jpginfo.prog"; break;
  default: tok = NULL;
  }
  if (tok)
  {
    if (buf[0] != '\0') strcat(buf, ", ");
    strcat(buf, message(tok));
  }

  /* append a string for the compression type */
  tok = (info->flags & jpeg_FLAG_ARITH) ? "jpginfo.arith" : "jpginfo.huff";  
  if (tok)
  {
    if (buf[0] != '\0') strcat(buf, ", ");
    strcat(buf, message0(tok));
  }

  err = image_set_info(image, image_INFO_FORMAT, buf);
  if (err)
    return err;

  /* record the colourspace and number of components */
  switch (info->colourspace)
  {
  case jpeg_COLOURSPACE_GREYSCALE: tok = "jpginfo.grey";  ncomps = 1; break;
  case jpeg_COLOURSPACE_RGB:       tok = "jpginfo.rgb";   ncomps = 3; break;
  case jpeg_COLOURSPACE_YCBCR:     tok = "jpginfo.ycbcr"; ncomps = 3; break;
  case jpeg_COLOURSPACE_CMYK:      tok = "jpginfo.cmyk";  ncomps = 4; break;
  case jpeg_COLOURSPACE_YCCK:      tok = "jpginfo.ycck";  ncomps = 4; break;
  default:
  case jpeg_COLOURSPACE_UNKNOWN:   tok = "jpginfo.unknown"; ncomps = 0; break;
  }
  err = image_set_info(image, image_INFO_COLOURSPACE, message0(tok));
  if (err)
    return err;

  err = image_set_info(image, image_INFO_NCOMPONENTS, &ncomps);
  if (err)
    return err;
  
  /* bits per component is always 8 presently */
  err = image_set_info(image, image_INFO_BPC, &bpc);
  if (err)
    return err;

  return result_OK;
}

static int jpeg_load(image_choices *choices, image_t *image)
{
  os_error       *e;
  unsigned char  *data;
  size_t          file_size;
  jpeg_info_t     info;
  jpeg_info_flags flags;
  int             width, height;
  int             xdpi, ydpi;
  int             bpp;

  /* init any fields used by the unload fn */
  image->image = NULL;

  osfile_read_no_path(image->file_name,
                     &image->source.load,
                     &image->source.exec,
             (int *) &image->source.file_size,
                      NULL /* attr */);

  file_size = image->source.file_size;

  if (flex_alloc((flex_ptr) &data, file_size) == 0)
    goto NoMem;

  e = EC(xosfile_load_stamped_no_path(image->file_name,
                                      data,
                                      NULL /* obj_type */,
                                      NULL /* load */,
                                      NULL /* exec */,
                                      NULL /* size */,
                                      NULL /* attr */));
  if (e == NULL)
  {
    jpeg_get_info(data, file_size, &info);
    if (choices->jpeg.cleaning == image_JPEG_CLEANING_ALWAYS ||
        !jpeg_supported(&info))
    {
      unsigned char *newdata;
      size_t         newlength;

      if (jpegtran_clean(data, file_size, &newdata, &newlength))
      {
        flex_free((flex_ptr) &data);
        oserror_report(0, "error.jpeg.transcode", jpegtran_get_messages());
        jpegtran_discard_messages();
        return TRUE; /* failure */
      }

      flex_free((flex_ptr) &data);

      flex_reanchor((flex_ptr) &data, (flex_ptr) &newdata);
      file_size = newlength;
    }

    e = EC(xjpeginfo_dimensions((jpeg_image *) data,
                                 file_size,
                                &flags,
                                &width,
                                &height,
                                &xdpi,
                                &ydpi,
                                 NULL));
  }

  if (e)
  {
    flex_free((flex_ptr) &data);
    oserror_report_block(e);
    return TRUE; /* failure */
  }

  if (flags & jpeg_INFO_MONOCHROME)
  {
    bpp = 8;
    image->flags = 0;
  }
  else
  {
    bpp = 24;
    image->flags = image_FLAG_COLOUR;
  }
  image->display.file_type = jpeg_FILE_TYPE;

  image->flags |= image_FLAG_CAN_ROT | image_FLAG_CAN_SPR;

  jpeg_populate_info(image, &info);

  flex_reanchor((flex_ptr) &image->image, (flex_ptr) &data);

  /* signal if metadata is available.
   * this has to wait until now that the anchor is in image->image. */

  if (jpeg_meta_available(image))
    image->flags |= image_FLAG_HAS_META;

  image->display.dims.bm.width  = width;
  image->display.dims.bm.height = height;
  image->display.dims.bm.xeig   = 1;
  image->display.dims.bm.yeig   = 1;
  image->display.dims.bm.xdpi   = xdpi;
  image->display.dims.bm.ydpi   = ydpi;
  image->display.dims.bm.bpp    = bpp;
  image->display.file_size      = file_size;

  image->source.dims = image->display.dims;
  image->source.dims.bm.xdpi    = xdpi;
  image->source.dims.bm.ydpi    = ydpi;
  /* load, exec, file_size set */
  image->source.file_type       = jpeg_FILE_TYPE;

  image->scale.min = 1;
  image->scale.max = 32767;

  if (choices->jpeg.sprite)
    return jpeg_to_spr_common(image);

  return FALSE; /* success */


NoMem:

  oserror_report(0, "error.no.mem");

  return TRUE; /* failure */
}

static int jpeg_unload(image_t *image)
{
  if (image->image)
    flex_free((flex_ptr) &image->image);

  return FALSE; /* success */
}

static int jpeg_rotate(image_choices *choices, image_t *image, int angle)
{
  jpegtran_transform_type    args;
  int              rc;
  unsigned char   *newdata;
  size_t           newlength;

  args = angle / 90; /* 0..719 -> 0..7 */

  if (choices->jpeg.trim)
    args |= jpegtran_TRANSFORM_FLAG_TRIM;

  rc = jpegtran_transform(image->image,
                          image->display.file_size,
                         &newdata,
                         &newlength,
                          args);
  if (rc)
  {
    return TRUE; /* failure */
  }

  flex_free((flex_ptr) &image->image);

  /* I used to have |if (args & 1)| here, reasoning that rotations which are
   * just flips preserve the same dimensions but that's not necessarily true
   * when the 'trim' flag is in effect. */

  {
    jpeg_info_flags flags;
    int             width, height;
    int             xdpi, ydpi;

    /* Rotate +/-90, Transpose, Transverse all change the image dimensions.
     * Also if -trim is enabled. In those cases, re-read them. */

    (void) EC(xjpeginfo_dimensions((jpeg_image *) newdata,
                                                  newlength,
                                                 &flags,
                                                 &width,
                                                 &height,
                                                 &xdpi,
                                                 &ydpi,
                                                  NULL));
    image->display.dims.bm.width  = width;
    image->display.dims.bm.height = height;
  }

  flex_reanchor((flex_ptr) &image->image, (flex_ptr) &newdata);

  image->display.file_size = newlength;

  image_modified(image, image_MODIFIED_DATA);

  return FALSE; /* success */
}

/* wrapper which signals modifications */
static int jpeg_to_spr(image_t *image)
{
  int rc;

  image_about_to_modify(image);

  rc = jpeg_to_spr_common(image);
  if (rc == FALSE)
  {
    image_modified(image, image_MODIFIED_FORMAT);
  }

  return rc;
}

static int jpeg_to_spr_common(image_t *image)
{
  static const image_methods methods =
  {
    jpeg_load,
    bitmap_save,
    bitmap_unload,
    bitmap_histogram,
    bitmap_rotate,
    NULL,
    NULL
  };

  os_error          *e;
  jpeg_info_flags    flags;
  int                width, height;
  int                xdpi, ydpi;
  int                bpp;
  os_mode            mode;
  int                sprrowbytes;
  int                sprpalbytes;
  int                sprimgbytes;
  osspriteop_area   *area;
  osspriteop_header *header;
  int                c0, c1, c2, c3;

  hourglass_on();

  e = EC(xjpeginfo_dimensions(image->image,
                              image->display.file_size,
                             &flags,
                             &width,
                             &height,
                             &xdpi,
                             &ydpi,
                              NULL));

  if (flags & jpeg_INFO_MONOCHROME)
  {
    bpp  = 8;
    mode = os_MODE8BPP90X90;
  }
  else
  {
    bpp  = 32;
    mode = (os_mode) ((osspriteop_TYPE32BPP << osspriteop_TYPE_SHIFT) |
                                        (90 << osspriteop_YRES_SHIFT) |
                                        (90 << osspriteop_XRES_SHIFT) |
                                               osspriteop_NEW_STYLE);
  }

  sprrowbytes = ((width * bpp + 31) & ~31) >> 3;
  if (bpp <= 8)
    sprpalbytes = 1 << (bpp + 3);
  else
    sprpalbytes = 0;
  sprimgbytes = 16 + 44 + sprpalbytes + sprrowbytes * height;

  if (flex_alloc((flex_ptr) &area, sprimgbytes) == 0)
    goto NoMem;

  area->size  = sprimgbytes;
  area->first = 16;
  osspriteop_clear_sprites(osspriteop_USER_AREA, area);

  osspriteop_create_sprite(osspriteop_NAME,
                           area,
                           "jpeg",
                           0, /* no palette */
                           width,
                           height,
                           mode);

  header = sprite_select(area, 0);

  if (bpp == 8)
  {
    /* Create a monochrome palette */

    unsigned int *palette;

    osspriteop_create_true_palette(osspriteop_PTR,
                                   area,
                   (osspriteop_id) header);

    palette = (unsigned int *) (header + 1);

    make_grey_palette(3, palette);
  }

  osspriteop_switch_output_to_sprite(osspriteop_PTR,
                                     area,
                     (osspriteop_id) header,
                                     NULL,
                                     &c0, &c1, &c2, &c3);

  e = xjpeg_plot_scaled(image->image,
                        0, 0,
                        NULL,
                        image->display.file_size,
                        0);

  if (e)
    oserror_plot(e, 0, 0);

  osspriteop_unswitch_output(c0, c1, c2, c3);

  flex_free((flex_ptr) &image->image);
  flex_reanchor((flex_ptr) &image->image, (flex_ptr) &area);

  image->flags = image_FLAG_COLOUR | image_FLAG_CAN_ROT;

  if (((unsigned int) mode >> osspriteop_TYPE_SHIFT) == osspriteop_TYPE32BPP)
    image->flags |= image_FLAG_CAN_HIST;

  image->display.dims.bm.xdpi = os_INCH >> 1;
  image->display.dims.bm.ydpi = os_INCH >> 1;
  image->display.dims.bm.bpp  = bpp;
  image->display.file_size    = sprimgbytes;
  image->display.file_type    = osfile_TYPE_SPRITE;

  image->details.sprite.mode  = mode;

  image->methods = methods;

  hourglass_off();

  return FALSE; /* success */


NoMem:

  hourglass_off();

  oserror_report(0, "error.no.mem");

  return TRUE; /* failure */
}

void jpeg_export_methods(image_choices *choices, image_t *image)
{
  static const image_methods methods =
  {
    jpeg_load,
    generic_save,
    jpeg_unload,
    NULL,
    jpeg_rotate,
    jpeg_get_meta,
    jpeg_to_spr,
  };

  NOT_USED(choices);

  image->methods = methods;
}
