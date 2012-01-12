/* --------------------------------------------------------------------------
 *    Name: tags.c
 * Purpose: Tags window
 * ----------------------------------------------------------------------- */

#ifdef EYE_TAGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/hourglass.h"
#include "oslib/osfile.h"
#include "oslib/osfscontrol.h"

#include "appengine/types.h"
#include "appengine/base/messages.h"
#include "appengine/base/errors.h"
#include "appengine/base/strings.h"
#include "appengine/databases/tag-db.h"
#include "appengine/databases/digest-db.h"
#include "appengine/gadgets/tag-cloud.h"
#include "appengine/graphics/image-observer.h"
#include "appengine/graphics/image.h"

#include "privateeye.h"
#include "actions.h"
#include "globals.h"
#include "keymap.h"
#include "actions.h"
#include "tags-common.h"

#include "tags.h"

/* ----------------------------------------------------------------------- */

static struct
{
  tag_cloud *tc;
  image_t   *image;
}
LOCALS;

/* ----------------------------------------------------------------------- */

/* Delete 'index'. */
static error deletetag(tag_cloud *tc,
                       int        index,
                       void      *arg)
{
  error err;

  err = tags_common__delete_tag(tc, index, arg);
  if (err)
    return err;

  err = tags_common__set_highlights(tc, LOCALS.image);
  if (err)
    return err;

  return error_OK;
}

static error tag(tag_cloud *tc,
                 int        index,
                 void      *arg)
{
  error         err;
  unsigned char digest[16];

  if (LOCALS.image == NULL)
    return error_OK;

  err = image_get_digest(LOCALS.image, digest);
  if (err)
    return err;

  err = tags_common__tag(tc, index, (char *) digest, LOCALS.image->file_name,
                         arg);
  if (err)
    return err;

  err = tags_common__set_highlights(tc, LOCALS.image);
  if (err)
    return err;

  return error_OK;
}

static error detag(tag_cloud *tc,
                   int        index,
                   void      *arg)
{
  error         err;
  unsigned char digest[16];

  if (LOCALS.image == NULL)
    return error_OK;

  err = image_get_digest(LOCALS.image, digest);
  if (err)
    return err;

  err = tags_common__detag(tc, index, (char *) digest, arg);
  if (err)
    return err;

  err = tags_common__set_highlights(tc, LOCALS.image);
  if (err)
    return err;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static tag_cloud_event keyhandler(wimp_key_no  key_no,
                                   void        *arg)
{
  int op;

  NOT_USED(arg);

  op = viewer_keymap_op(viewer_keymap_SECTION_TAG_CLOUD, key_no);

  switch (op)
  {
    case TagCloud_List:         return tag_cloud_EVENT_DISPLAY_LIST;
    case TagCloud_Cloud:        return tag_cloud_EVENT_DISPLAY_CLOUD;

    case TagCloud_SortByName:   return tag_cloud_EVENT_SORT_BY_NAME;
    case TagCloud_SortByCount:  return tag_cloud_EVENT_SORT_BY_COUNT;

    case TagCloud_SortSelFirst: return tag_cloud_EVENT_SORT_SELECTED_FIRST;

    case TagCloud_Rename:       return tag_cloud_EVENT_RENAME;
    case TagCloud_Kill:         return tag_cloud_EVENT_KILL;
    case TagCloud_New:          return tag_cloud_EVENT_NEW;
    case TagCloud_Info:         return tag_cloud_EVENT_INFO;

    case TagCloud_Commit:       return tag_cloud_EVENT_COMMIT;

    case Close:                 return tag_cloud_EVENT_CLOSE;

    case Help:
      action_help();
      break;
  }

  return tag_cloud_EVENT_UNKNOWN;
}

/* ----------------------------------------------------------------------- */

static void tags__image_changed_callback(image_t              *image,
                                         imageobserver_change  change,
                                         imageobserver_data   *data,
                                         void                 *opaque)
{
  error err;

  /* remember that this gets called even when the tags window is closed */

  NOT_USED(data);
  NOT_USED(opaque);

  switch (change)
  {
  case imageobserver_CHANGE_GAINED_FOCUS:
    if (LOCALS.image != image)
    {
      char        scratch[32];
      const char *leaf;
      char        title[256];

      /* set its title, including the leafname of the image */

      sprintf(scratch, "tagcloud.title");
      leaf = str_leaf(image->file_name);
      sprintf(title, message0(scratch), leaf);
      window_set_title_text(tag_cloud_get_window_handle(LOCALS.tc), title);

      tag_cloud_shade(LOCALS.tc, 0);

      err = tags_common__set_highlights(LOCALS.tc, image);
      if (err)
        goto failure;

      /* remember the most recently focused image */
      LOCALS.image = image;
    }
    break;

  case imageobserver_CHANGE_LOST_FOCUS:
    /* We don't set LOCALS.image to NULL here because when _we_ receive the
     * the focus, this event is delivered. So we store the previously-focused
     * image. */
    break;

  case imageobserver_CHANGE_HIDDEN:
  case imageobserver_CHANGE_ABOUT_TO_DESTROY:
    if (LOCALS.image == image)
    {
      char        scratch[32];
      const char *leaf;
      char        title[256];

      /* set its title, including the leafname of the image */

      sprintf(scratch, "tagcloud.title");
      leaf = "(none)";
      sprintf(title, message0(scratch), leaf);
      window_set_title_text(tag_cloud_get_window_handle(LOCALS.tc), title);

      LOCALS.image = NULL;

      tags_common__clear_highlights(LOCALS.tc);

      tag_cloud_shade(LOCALS.tc, 1);
    }
    break;
  }

  return;


failure:

  error_report(err);

  return;
}

/* ----------------------------------------------------------------------- */

error tags__choices_updated(const choices_group *g)
{
  tag_cloud_config c;

  NOT_USED(g);

  if (LOCALS.tc == NULL)
    return error_OK;

  c.size    = GLOBALS.choices.tagcloud.size;
  c.leading = GLOBALS.choices.tagcloud.leading;
  c.padding = GLOBALS.choices.tagcloud.padding;
  c.scale   = GLOBALS.choices.tagcloud.scale;

  tag_cloud_set_config(LOCALS.tc, &c);

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static void tags__properfin(int force);

/* ----------------------------------------------------------------------- */

static int tags__refcount = 0;

error tags__init(void)
{
  if (tags__refcount++ == 0)
  {
    error err;

    /* dependencies */

    err = viewer_keymap_init();
    if (err)
      return err;

    err = tags_common__init();
    if (err)
      return err;
  }

  return error_OK;
}

void tags__fin(void)
{
  if (--tags__refcount == 0)
  {
    tags__properfin(1); /* force shutdown */

    tags_common__fin();

    viewer_keymap_fin();
  }
}

/* ----------------------------------------------------------------------- */

/* The 'proper' init/fin functions provide lazy initialisation. */

static int tags__properrefcount = 0;

static error tags__properinit(void)
{
  error err;

  if (tags__properrefcount++ == 0)
  {
    tag_cloud_config  conf;
    tag_cloud         *tc = NULL;
    tagdb             *db = NULL;

    err = tags_common__properinit();
    if (err)
      return err;

    conf.size    = GLOBALS.choices.tagcloud.size;
    conf.leading = GLOBALS.choices.tagcloud.leading;
    conf.padding = GLOBALS.choices.tagcloud.padding;
    conf.scale   = GLOBALS.choices.tagcloud.scale;

    tc = tag_cloud_create(0 /* flags */, &conf);
    if (tc == NULL)
    {
      err = error_OOM;
      goto Failure;
    }

    imageobserver_register_greedy(tags__image_changed_callback, NULL);

    db = tags_common__get_db(); /* FIXME: Feels a bit grotty. */

    tag_cloud_set_handlers(tc,
                            tags_common__add_tag,
                            deletetag,
                            tags_common__rename_tag,
                            tag,
                            detag,
                            tags_common__tagfile,
                            tags_common__detagfile,
                            tags_common__event,
                            db);

    tag_cloud_set_key_handler(tc, keyhandler, db);

    tag_cloud_set_display(tc, GLOBALS.choices.tagcloud.display);
    tag_cloud_set_sort(tc, GLOBALS.choices.tagcloud.sort);
    tag_cloud_set_order(tc, GLOBALS.choices.tagcloud.selfirst);

    LOCALS.tc = tc;
  }

  return error_OK;

Failure:

  return err;
}

/* This is only ever called with force set true at the moment. */
static void tags__properfin(int force)
{
  if (tags__properrefcount == 0)
    return;

  if (force)
    tags__properrefcount = 1;

  if (--tags__properrefcount == 0)
  {
    imageobserver_deregister_greedy(tags__image_changed_callback, NULL);

    tag_cloud_destroy(LOCALS.tc);

    tags_common__properfin(0); /* don't pass 'force' in */
  }
}

/* ----------------------------------------------------------------------- */

error tags__open(image_t *image)
{
  error             err;
  wimp_window_state state;

  /* load the databases, create tag cloud, install image observer */

  err = tags__properinit();
  if (err)
    return err;

  state.w = tag_cloud_get_window_handle(LOCALS.tc);
  wimp_get_window_state(&state);

  if (state.flags & wimp_WINDOW_OPEN)
  {
    /* bring to front and gain focus */

    state.next = wimp_TOP;
    wimp_open_window((wimp_open *) &state);
  }
  else
  {
    char        scratch[32];
    const char *leaf;
    char        title[256];

    /* set its title, including the leafname of the image */

    sprintf(scratch, "tagcloud.title");
    leaf = str_leaf(image->file_name);
    sprintf(title, message0(scratch), leaf);
    window_set_title_text(tag_cloud_get_window_handle(LOCALS.tc), title);

    /* opening for the first(?) time */

    err = tags_common__set_tags(LOCALS.tc);
    if (err)
      goto failure;

    /* FIXME: This duplicates code in tags__image_changed_callback. */
    err = tags_common__set_highlights(LOCALS.tc, image);
    if (err)
      goto failure;

    LOCALS.image = image;

    tag_cloud_open(LOCALS.tc);
  }

  /* FIXME: This doesn't work - needs investigation. */
  /* wimp_set_caret_position(state.w, wimp_ICON_WINDOW, -1, -1, -1, -1); */

  return error_OK;


failure:

  return err;
}

#else

extern int dummy;

#endif /* EYE_TAGS */
