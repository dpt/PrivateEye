/* --------------------------------------------------------------------------
 *    Name: info.c
 * Purpose: Info
 * Version: $Id: info.c,v 1.23 2009-05-20 21:38:18 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <float.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "oslib/os.h"
#include "oslib/territory.h"
#include "oslib/wimp.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/dialogues/info.h"
#include "appengine/base/messages.h"
#include "appengine/base/numstr.h"

#include "globals.h"
#include "viewer.h"

#include "info.h"

/* ----------------------------------------------------------------------- */

/* The image and source info windows are very similar, so the common parts
 * are factored out into this structure. */

typedef struct InfoDialogueSpecifier
{
  int       icon_date;
  ptrdiff_t attrs;
  ptrdiff_t file_type;
  ptrdiff_t file_size;
}
InfoDialogueSpecifier;

static void populate_info_dialogue(dialogue_t *d, const InfoDialogueSpecifier *info)
{
  static const char DASH[] = "˜";

  viewer     *viewer;
  image      *image;
  int         spec;
  char        buffer[1024]; /* Careful Now */
  char       *buf;
  bits        file_type;
  info_spec_t specs[32];    /* Careful Now */
  size_t      file_size;
  char        scratch[256]; /* Careful Now */
  bits        time[2];

  viewer = viewer_find(GLOBALS.current_display_w);
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

    attrs = (image_attrs *)((char *) image + info->attrs);

    /* dimensions */

    w = attrs->dims.bm.width;
    h = attrs->dims.bm.height;

    comma_number(w, scratch, 12);
    comma_number(h, scratch + 12, 12);
    comma_number(w * h, scratch + 24, 12);

    specs[spec++].value = buf;
    buf += sprintf(buf, message0("size.pixels"),
                   scratch, scratch + 12, scratch + 24) + 1;

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

      comma_double((double) w / xdpi, scratch, sizeof(scratch));
      comma_double((double) h / ydpi, scratch + 12, sizeof(scratch) - 12);

      specs[spec++].value = buf;
      buf += sprintf(buf, message0("size.inches"), scratch, scratch + 12) + 1;
    }

    /* depth, mask, colour */

    specs[spec++].value = buf;

    sprintf(buf, message0("info.depth"), attrs->dims.bm.bpp);

    strcat(buf, message0((image->flags & image_FLAG_COLOUR) ?
           "info.colour" : "info.mono"));

    if (image->flags & image_FLAG_HAS_ALPHA)
      strcat(buf, message0("info.alpha"));

    if (image->flags & image_FLAG_HAS_MASK)
      strcat(buf, message0("info.mask"));

    buf += strlen(buf) + 1;
  }

  if (info->icon_date > -1)
  {
    specs[spec++].value = buf;

    time[0] = image->source.exec;
    time[1] = image->source.load;
    territory_convert_standard_date_and_time(territory_CURRENT,
                  (const os_date_and_time *) time,
                                             scratch,
                                             sizeof(scratch));

    buf += sprintf(buf, "%s", scratch) + 1;
  }

  file_type = *((size_t *) ((char *) image + info->file_type));

  info__set_file_type(d, file_type);
  info__set_info(d, specs, spec);

  info__layout(d);
}

/* ----------------------------------------------------------------------- */

dialogue_t *info;

static void info_fillout(dialogue_t *d, void *arg)
{
  static const InfoDialogueSpecifier info =
  {
    -1, /* no icon for date */
    offsetof(image, display),
    offsetof(image, display.file_type),
    offsetof(image, display.file_size)
  };

  NOT_USED(arg);

  populate_info_dialogue(d, &info);
}

/* ----------------------------------------------------------------------- */

dialogue_t *source_info;

static void source_info_fillout(dialogue_t *d, void *arg)
{
  static const InfoDialogueSpecifier info =
  {
    0, /* date icon present */
    offsetof(image, source),
    offsetof(image, source.file_type),
    offsetof(image, source.file_size)
  };

  NOT_USED(arg);

  populate_info_dialogue(d, &info);
}

/* ----------------------------------------------------------------------- */

error viewer_info_init(void)
{
  /* Info dialogue */

  info = info__create("image_info");
  if (info == NULL)
    return error_OOM;

  dialogue__set_fillout_handler(info, info_fillout, NULL);

  info__set_padding(info, GLOBALS.choices.info.padding);

  /* Source Info dialogue */

  source_info = info__create("source_info");
  if (source_info == NULL)
    return error_OOM;

  dialogue__set_fillout_handler(source_info, source_info_fillout, NULL);

  info__set_padding(source_info, GLOBALS.choices.info.padding);

  return error_OK;
}

void viewer_info_fin(void)
{
  info__destroy(source_info);
  info__destroy(info);
}
