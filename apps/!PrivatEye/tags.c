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
#include "appengine/app/wire.h"
#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
#include "appengine/base/strings.h"
#include "appengine/databases/digest-db.h"
#include "appengine/databases/tag-db.h"
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
  tags_common       common;
  tag_cloud        *tc;
  image_t          *image;     /* current image */
  viewer_keymap_id  keymap_id;
}
LOCALS;

/* ---------------------------------------------------------------------- */

/* key defns */
enum
{
  TagCloud_List,
  TagCloud_Cloud,
  TagCloud_SmallCloud,
  TagCloud_UnscaledCloud,
  TagCloud_SortByCount,
  TagCloud_SortByName,
  TagCloud_SortSelFirst,
  TagCloud_Rename,
  TagCloud_Kill,
  TagCloud_New,
  TagCloud_Info,
  TagCloud_Commit,
};

/* ---------------------------------------------------------------------- */

static error declare_keymap(void)
{
  /* Keep these sorted by name */
  static const keymap_name_to_action keys[] =
  {
    { "Cloud",            TagCloud_Cloud          },
    { "Commit",           TagCloud_Commit         },
    { "Info",             TagCloud_Info           },
    { "Kill",             TagCloud_Kill           },
    { "List",             TagCloud_List           },
    { "New",              TagCloud_New            },
    { "Rename",           TagCloud_Rename         },
    { "SmallCloud",       TagCloud_SmallCloud     },
    { "SortByCount",      TagCloud_SortByCount    },
    { "SortByName",       TagCloud_SortByName     },
    { "SortSelFirst",     TagCloud_SortSelFirst   },
    { "UnscaledCloud",    TagCloud_UnscaledCloud  },
  };

  return viewer_keymap_add("Tag Cloud",
                           keys,
                           NELEMS(keys),
                           &LOCALS.keymap_id);
}

static error tags_substrate_callback(const wire_message_t *message,
                                     void                 *opaque)
{
  NOT_USED(opaque);

  switch (message->event)
  {
    case wire_event_DECLARE_KEYMAP:
      return declare_keymap();
  }

  return error_OK;
}

error tags_substrate_init(void)
{
  error err;

  err = wire_register(0, tags_substrate_callback, NULL, NULL);
  if (err)
    return err;

  return error_OK;
}

/* ---------------------------------------------------------------------- */

/* Delete 'index'. */
static error deletetag(tag_cloud *tc, int index, void *opaque)
{
  error err;

  err = tags_common_delete_tag(tc, index, opaque);
  if (err)
    return err;

  err = tags_common_set_highlights(tc, LOCALS.image, &LOCALS.common);
  if (err)
    return err;

  return error_OK;
}

static error tag(tag_cloud *tc, int index, void *opaque)
{
  error         err;
  unsigned char digest[16];

  if (LOCALS.image == NULL)
    return error_OK;

  err = image_get_digest(LOCALS.image, digest);
  if (err)
    return err;

  err = tags_common_tag(tc,
                         index,
                (char *) digest,
                         LOCALS.image->file_name,
                         opaque);
  if (err)
    return err;

  err = tags_common_set_highlights(tc, LOCALS.image, &LOCALS.common);
  if (err)
    return err;

  return error_OK;
}

static error detag(tag_cloud *tc, int index, void *opaque)
{
  error         err;
  unsigned char digest[16];

  if (LOCALS.image == NULL)
    return error_OK;

  err = image_get_digest(LOCALS.image, digest);
  if (err)
    return err;

  err = tags_common_detag(tc, index, (char *) digest, opaque);
  if (err)
    return err;

  err = tags_common_set_highlights(tc, LOCALS.image, &LOCALS.common);
  if (err)
    return err;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static tag_cloud_event keyhandler(wimp_key_no key_no, void *opaque)
{
  int op;

  NOT_USED(opaque);

  op = viewer_keymap_op(LOCALS.keymap_id, key_no);

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

static void tags_image_changed_callback(image_t             *image,
                                         imageobserver_change change,
                                         imageobserver_data  *data,
                                         void                *opaque)
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
      // FIXME: Hoist this and common code below to a subroutine.
      sprintf(scratch, "tagcloud.title");
      leaf = str_leaf(image->file_name);
      sprintf(title, message0(scratch), leaf);
      window_set_title_text(tag_cloud_get_window_handle(LOCALS.tc), title);

      tag_cloud_shade(LOCALS.tc, 0);

      err = tags_common_set_highlights(LOCALS.tc, image, &LOCALS.common);
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

      tags_common_clear_highlights(LOCALS.tc);

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

// we receive this callback when choices are updated, but tag-search does not.
// choices only has a single callback, so either that must dispatch to, say,
// tags_common and be divided out there, or choices could be amended to output change notfications on
// the wire system.

error tags_choices_updated(const choices_group *group)
{
  tag_cloud_config c;

  NOT_USED(group);

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

static void tags_lazyfin(int force);

/* ----------------------------------------------------------------------- */

static unsigned int tags_refcount = 0;

error tags_init(void)
{
  error err;

  if (tags_refcount == 0)
  {
    /* dependencies */

    err = viewer_keymap_init();
    if (err)
      return err;

    err = tags_common_init();
    if (err)
      return err;
  }

  tags_refcount++;

  return error_OK;
}

void tags_fin(void)
{
  if (tags_refcount == 0)
    return;

  if (--tags_refcount == 0)
  {
    /* finalise */

    tags_lazyfin(1); /* force shutdown */

    /* dependencies */

    tags_common_fin();

    viewer_keymap_fin();
  }
}

/* ----------------------------------------------------------------------- */

/* The 'lazy' init/fin functions provide lazy initialisation. */

static int tags_lazyrefcount = 0;

static error tags_lazyinit(void)
{
  error err;

  if (tags_lazyrefcount++ == 0)
  {
    tag_cloud_config conf;
    tag_cloud       *tc = NULL;

    err = tags_common_lazyinit();
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

    imageobserver_register_greedy(tags_image_changed_callback, NULL);

    tag_cloud_set_handlers(tc,
                           tags_common_add_tag,
                           deletetag,
                           tags_common_rename_tag,
                           tag,
                           detag,
                           tags_common_tagfile,
                           tags_common_detagfile,
                           tags_common_event,
                           &LOCALS.common);

    tag_cloud_set_key_handler(tc, keyhandler, &LOCALS.common);

    tag_cloud_set_display(tc, GLOBALS.choices.tagcloud.display);
    tag_cloud_set_sort(tc, GLOBALS.choices.tagcloud.sort);
    tag_cloud_set_order(tc, GLOBALS.choices.tagcloud.selfirst);

    LOCALS.common.db          = tags_common_get_db(); /* FIXME: Seems grotty. */
    LOCALS.common.indextotag  = NULL;
    LOCALS.common.nindextotag = 0;

    LOCALS.tc                 = tc;
    LOCALS.image              = NULL;
  }

  return error_OK;


Failure:

  return err;
}

/* This is only ever called with force set true at the moment. */
static void tags_lazyfin(int force)
{
  if (tags_lazyrefcount == 0)
    return;

  if (force)
    tags_lazyrefcount = 1;

  if (--tags_lazyrefcount == 0)
  {
    imageobserver_deregister_greedy(tags_image_changed_callback, NULL);

    free(LOCALS.common.indextotag);

    tag_cloud_destroy(LOCALS.tc);

    tags_common_lazyfin(0); /* don't pass 'force' in */
  }
}

/* ----------------------------------------------------------------------- */

error tags_open(image_t *image)
{
  error             err;
  wimp_window_state state;

  /* load the databases, create tag cloud, install image observer */

  err = tags_lazyinit();
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

    // FIXME: Hoist to tags_set_title. See above.
    sprintf(scratch, "tagcloud.title");
    leaf = str_leaf(image->file_name);
    sprintf(title, message0(scratch), leaf);
    window_set_title_text(tag_cloud_get_window_handle(LOCALS.tc), title);

    /* opening for the first(?) time */

    err = tags_common_set_tags(LOCALS.tc, &LOCALS.common);
    if (err)
      goto failure;

    /* FIXME: This duplicates code in tags_image_changed_callback. */
    err = tags_common_set_highlights(LOCALS.tc, image, &LOCALS.common);
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
