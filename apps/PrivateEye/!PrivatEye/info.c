/* --------------------------------------------------------------------------
 *    Name: info.c
 * Purpose: Viewer info dialogue handler
 * ----------------------------------------------------------------------- */

#include <float.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "oslib/os.h"
#include "oslib/territory.h"
#include "oslib/wimp.h"

#include "appengine/base/messages.h"
#include "appengine/base/numstr.h"
#include "appengine/dialogues/info.h"
#include "appengine/wimp/dialogue.h"

#include "globals.h"
#include "viewer.h"

#include "info.h"

/* ----------------------------------------------------------------------- */

/* The image and source info windows are very similar, so the common parts
 * are factored out into this structure. */

typedef struct InfoDialogueSpecifier
{
  int       is_source;
  ptrdiff_t attrs;
  ptrdiff_t file_type;
  ptrdiff_t file_size;
}
InfoDialogueSpecifier;

static void populate_info_dialogue(dialogue_t                  *d,
                                   const InfoDialogueSpecifier *info)
{
  static const char DASH[] = "�";

  viewer_t   *viewer;
  image_t    *image;
  int         spec;
  char        buffer[1024]; /* Careful Now */
  char       *buf;
  bits        file_type;
  info_spec_t specs[32];    /* Careful Now */
  size_t      file_size;
  char        scratch[256]; /* Careful Now */
  bits        time[2];

  viewer = GLOBALS.current_viewer;
  if (viewer == NULL)
    return;

  image = viewer->drawable->image;

  spec = 0;

  buf = buffer;

  /* file size */

  file_size = *((size_t *) ((char *) image + info->file_size));

  comma_number(file_size, scratch, sizeof(scratch));
  specs[spec++].value = buf;
  buf += sprintf(buf, message0("size.bytes"), scratch) + 1;

  /* attributes */

  if (image->flags & image_FLAG_VECTOR)
  {
    /* these all point to the same string */
    specs[spec++].value = DASH;
    specs[spec++].value = DASH;
    specs[spec++].value = DASH;
    specs[spec++].value = DASH;
  }
  else
  {
    image_attrs *attrs;
    int          w,h;
    int          xdpi,ydpi;

    // FIXME: This mush looks like a strong argument for better notation and
    //        a custom printf-style formatting scheme.

    attrs = (image_attrs *)((char *) image + info->attrs);

    /* dimensions */

    w = attrs->dims.bm.width;
    h = attrs->dims.bm.height;

    comma_number(w, scratch, 12);
    comma_number(h, scratch + 12, 12);
    comma_number(w * h, scratch + 24, 12);

    specs[spec++].value = buf;
    buf += sprintf(buf,
                   message0("size.pixels"),
                   scratch,
                   scratch + 12,
                   scratch + 24) + 1;

    xdpi = attrs->dims.bm.xdpi;
    ydpi = attrs->dims.bm.ydpi;

    /* dpi */

    if (xdpi == 0 || ydpi == 0)
    {
      specs[spec++].value = buf;
      buf += sprintf(buf, message0("unspec")) + 1;

      specs[spec++].value = DASH;
    }
    else
    {
      specs[spec++].value = buf;
      buf += sprintf(buf, message0("size.dpi"), xdpi, ydpi) + 1;

      double wdpi_in, hdpi_in, wdpi_cm, hdpi_cm;

      wdpi_in = (double) w / xdpi;
      hdpi_in = (double) h / ydpi;
      wdpi_cm = wdpi_in * 2.54;
      hdpi_cm = hdpi_in * 2.54;

      comma_double(wdpi_cm, scratch +  0, 12);
      comma_double(hdpi_cm, scratch + 12, 12);
      comma_double(wdpi_in, scratch + 24, 12);
      comma_double(hdpi_in, scratch + 36, 12);

      specs[spec++].value = buf;
      buf += sprintf(buf, message0("size.physical"),
                     scratch,
                     scratch + 12,
                     scratch + 24,
                     scratch + 36) + 1;
    }

    /* depth, colour, alpha, mask */

    specs[spec++].value = buf;

    sprintf(buf, message0("info.depth"), attrs->dims.bm.bpp);

    /* test in order CMYK then COLOUR, otherwise it's monochrome */
    if (image->flags & image_FLAG_CMYK)
      strcat(buf, message0("info.cmyk"));
    else if (image->flags & image_FLAG_COLOUR)
      strcat(buf, message0("info.colour"));
    else
      strcat(buf, message0("info.mono"));

    if (image->flags & image_FLAG_HAS_ALPHA)
      strcat(buf, message0("info.alpha.chan"));
    if (image->flags & image_FLAG_HAS_ALPHA_MASK)
      strcat(buf, message0("info.alpha.mask"));
    else if (image->flags & image_FLAG_HAS_MASK)
      strcat(buf, message0("info.mask"));

    buf += strlen(buf) + 1;
  }

  if (!info->is_source)
  {
    int                  present;
    const char          *format;

    /* set format */
    present = image_get_info(image, image_INFO_DISPLAY_FORMAT, (void **) &format);
    specs[spec++].value = (present) ? format : DASH;
  }
  else
  {
    int                  present;
    const char          *format;
    const char          *space;
    const unsigned char *bpc;
    const unsigned char *ncomps;

    /* decode and set date and time */
    time[0] = image->source.exec;
    time[1] = image->source.load;
    territory_convert_standard_date_and_time(territory_CURRENT,
                  (const os_date_and_time *) time,
                                             scratch,
                                             sizeof(scratch));
    specs[spec++].value = buf;
    buf += sprintf(buf, "%s", scratch) + 1;
    
    /* set format */
    present = image_get_info(image, image_INFO_FORMAT, (void **) &format);
    specs[spec++].value = (present) ? format : DASH;

    /* set (override) colours field */
    present = image_get_info(image, image_INFO_COLOURSPACE, (void **) &space);
    if (present) present = image_get_info(image, image_INFO_BPC, (void **) &bpc);
    if (present) present = image_get_info(image, image_INFO_NCOMPONENTS, (void **) &ncomps);
    if (present)
    {
      specs[4].value = buf;
      buf += sprintf(buf, message0("info.extdcolour"), space, *ncomps, *bpc) + 1;
    }
  }

  file_type = *((size_t *) ((char *) image + info->file_type));

  info_set_file_type(d, file_type);
  info_set_info(d, specs, spec);

  info_layout(d);
}

/* ----------------------------------------------------------------------- */

dialogue_t *viewer_infodlg;

static void viewer_infodlg_fillout(dialogue_t *d, void *opaque)
{
  static const InfoDialogueSpecifier info =
  {
    0, /* is not source */
    offsetof(image_t, display),
    offsetof(image_t, display.file_type),
    offsetof(image_t, display.file_size)
  };

  NOT_USED(opaque);

  populate_info_dialogue(d, &info);
}

/* ----------------------------------------------------------------------- */

dialogue_t *viewer_srcinfodlg;

static void viewer_srcinfodlg_fillout(dialogue_t *d, void *opaque)
{
  static const InfoDialogueSpecifier info =
  {
    1, /* is source */
    offsetof(image_t, source),
    offsetof(image_t, source.file_type),
    offsetof(image_t, source.file_size)
  };

  NOT_USED(opaque);

  populate_info_dialogue(d, &info);
}

/* ----------------------------------------------------------------------- */

result_t viewer_infodlg_init(void)
{
  // FIXME: Idea for data driving this process.
  //
  // static const struct
  // {
  //   const char               *name;
  //   wimp_w                   *w;
  //   dialogue_fillout_handler *fillout;
  // }
  // map[] =
  // {
  //   { "image_info",  &viewer_infodlg,    viewer_infodlg_fillout    },
  //   { "source_info", &viewer_srcinfodlg, viewer_srcinfodlg_fillout }
  // };
  //
  // int i;
  //
  // for (i = 0; i < NELEMS(map); i++)
  // {
  //   *map[i].w = info_create(map[i].name);
  //   if (*map[i].w == NULL)
  //     return result_OOM; // will leak when i > 0
  //
  //   dialogue_set_fillout_handler(*map[i].w, map[i].fillout, NULL);
  //
  //   info_set_padding(*map[i].w, GLOBALS.choices.info.padding);
  // }
  //
  // return result_OK;

  /* Info dialogue */

  viewer_infodlg = info_create("image_info");
  if (viewer_infodlg == NULL)
    return result_OOM;

  dialogue_set_fillout_handler(viewer_infodlg,
                               viewer_infodlg_fillout,
                               NULL);

  info_set_padding(viewer_infodlg, GLOBALS.choices.info.padding);

  /* Source Info dialogue */

  viewer_srcinfodlg = info_create("source_info");
  if (viewer_srcinfodlg == NULL)
    return result_OOM;

  dialogue_set_fillout_handler(viewer_srcinfodlg,
                               viewer_srcinfodlg_fillout,
                               NULL);

  info_set_padding(viewer_srcinfodlg, GLOBALS.choices.info.padding);

  return result_OK;
}

void viewer_infodlg_fin(void)
{
  info_destroy(viewer_srcinfodlg);
  info_destroy(viewer_infodlg);
}
