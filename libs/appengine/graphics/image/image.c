/* --------------------------------------------------------------------------
 *    Name: image.c
 * Purpose: Image block handling
 * Version: $Id: image.c,v 1.2 2010-01-09 22:06:01 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "md5/md5.h"

#include "oslib/types.h"
#include "oslib/osfile.h"

#include "appengine/graphics/image-observer.h"
#include "appengine/io/md5.h"

#include "formats.h"

#include "appengine/graphics/image.h"

static list_t list_anchor = { NULL };

/* ----------------------------------------------------------------------- */

static void image_reset(image *i)
{
  /* dispose of histogram data */
  free(i->hists);
  i->hists = NULL;
}

/* ----------------------------------------------------------------------- */

image *image_create(void)
{
  image *i;

  i = calloc(1, sizeof(*i));
  if (i == NULL)
    return i;

  i->refcount = 1;
  i->background_colour = os_COLOUR_TRANSPARENT;

  list__add_to_head(&list_anchor, &i->list);

  return i;
}

image *image_create_from_file(image_choices *choices, const char *file_name, bits file_type)
{
  image *i;

  i = image_create();
  if (i == NULL)
      goto Failure;

  strcpy(i->file_name, file_name);
  i->source.file_type = file_type;

  if (loader_export_methods(choices, i, file_type))
    goto Failure;

  if (i->methods.load(choices, i))
    goto Failure;

  return i;


Failure:

  image_destroy(i);

  return NULL;
}

/* ----------------------------------------------------------------------- */

void image_addref(image *i)
{
  i->refcount++;
}

void image_deleteref(image *i)
{
  i->refcount--;
}

/* ----------------------------------------------------------------------- */

int image_start_editing(image *i)
{
  if (i->flags & image_FLAG_EDITING)
    return 1;

  i->flags |= image_FLAG_EDITING;

  return 0;
}

void image_stop_editing(image *i)
{
  i->flags &= ~image_FLAG_EDITING;
}

osbool image_is_editing(const image *i)
{
  return (i->flags & image_FLAG_EDITING) != 0;
}

/* ----------------------------------------------------------------------- */

void image_preview(image *i)
{
  imageobserver_event(i, imageobserver_CHANGE_PREVIEW, NULL);
}

/* ----------------------------------------------------------------------- */

void image_hide(image *i)
{
  imageobserver_event(i, imageobserver_CHANGE_HIDDEN, NULL);
}

void image_reveal(image *i)
{
  imageobserver_event(i, imageobserver_CHANGE_REVEALED, NULL);
}

/* ----------------------------------------------------------------------- */

void image_focus(image *i)
{
  imageobserver_event(i, imageobserver_CHANGE_GAINED_FOCUS, NULL);
}

void image_defocus(image *i)
{
  imageobserver_event(i, imageobserver_CHANGE_LOST_FOCUS, NULL);
}

/* ----------------------------------------------------------------------- */

void image_about_to_modify(image *i)
{
  imageobserver_event(i, imageobserver_CHANGE_ABOUT_TO_MODIFY, NULL);
}

void image_modified(image *i, image_modified_flags flags)
{
  imageobserver_data data;

  i->flags |= image_FLAG_MODIFIED;

  image_reset(i);

  data.modified.flags = flags;

  imageobserver_event(i, imageobserver_CHANGE_MODIFIED, &data);
}

/* ----------------------------------------------------------------------- */

void image_destroy(image *i)
{
  if (i == NULL)
    return;

  if (--i->refcount <= 0) /* refcount may be zero to start with */
  {
    imageobserver_event(i, imageobserver_CHANGE_ABOUT_TO_DESTROY, NULL);

    list__remove(&list_anchor, &i->list);

    if (i->methods.unload)
      i->methods.unload(i);

    image_reset(i);

    free(i);
  }
}

/* ----------------------------------------------------------------------- */

void image_select(image *i, int index)
{
  /* Only sprites can have their index set. */
  if (i->display.file_type == osfile_TYPE_SPRITE)
    /* FIXME: Bounds check */
    i->details.sprite.index = index;
}

/* ----------------------------------------------------------------------- */

void image_map(image_map_callback *fn, void *arg)
{
  /* Note that the callback signatures are identical, so we can cast. */

  list__walk(&list_anchor, (list__walk_callback *) fn, arg);
}

/* ----------------------------------------------------------------------- */

static void count_callback(image *image, void *arg)
{
  int *i = arg;

  NOT_USED(image);

  (*i)++;
}

int image_get_count(void)
{
  int count;

  image_map(count_callback, &count);

  return count;
}

/* ----------------------------------------------------------------------- */

static error destroy_metadata_callback(ntree_t *t, void *arg)
{
  NOT_USED(arg);

  free(ntree__get_data(t));

  return error_OK;
}

void image_destroy_metadata(ntree_t *metadata)
{
  error err;

  err = ntree__walk(metadata,
                    ntree__WALK_POST_ORDER | ntree__WALK_ALL,
                    0,
                    destroy_metadata_callback,
                    NULL);

  ntree__delete(metadata);
}

/* ----------------------------------------------------------------------- */

error image_get_md5(image *image, char *digest)
{
  if ((image->flags & image_FLAG_HAS_DIGEST) == 0)
  {
    md5__from_file(image->file_name, image->digest);

    image->flags |= image_FLAG_HAS_DIGEST;
  }

  strcpy(digest, image->digest);

  return error_OK;
}
