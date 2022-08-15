/* --------------------------------------------------------------------------
 *    Name: image.c
 * Purpose: Image block handling
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/osfile.h"

#include "datastruct/list.h"
#include "utils/array.h"

#include "appengine/graphics/image-observer.h"
#include "appengine/io/md5.h"

#include "formats.h"

#include "appengine/graphics/image.h"

static list_t list_anchor = { NULL };

/* ----------------------------------------------------------------------- */

static void image_init_info(image_t *image)
{
  image->info.entries          = NULL;
  image->info.entriesused      = 0;
  image->info.entriesallocated = 0;

  image->info.data             = NULL;
  image->info.dataused         = 0;
  image->info.dataallocated    = 0;
}

static void image_destroy_info(image_t *image)
{
  free(image->info.entries);
  free(image->info.data);
  image_init_info(image);
}

static int image_ensure_entries(image_t *image)
{
  return array_grow((void **) &image->info.entries,
                               sizeof(*image->info.entries),
                               image->info.entriesused,
                              &image->info.entriesallocated,
                               1 /* need */,
                               4 /* minimum */);
}

static int image_ensure_data(image_t *image, int bytes_needed)
{
  return array_grow((void **) &image->info.data,
                               sizeof(*image->info.data),
                               image->info.dataused,
                              &image->info.dataallocated,
                               bytes_needed,
                               64 /* minimum */);
}

result_t image_set_info(image_t        *image,
                        image_info_key  key,
                        const void     *data)
{
  image_info *entry;
  size_t      nbytes;

  if (image_ensure_entries(image))
    return result_OOM;

  entry = &image->info.entries[image->info.entriesused++];

  switch (key)
  {
  case image_INFO_BPC:
  case image_INFO_NCOMPONENTS:
    nbytes = 1;
    break;
  case image_INFO_FORMAT:
  case image_INFO_COLOURSPACE:
    nbytes = strlen(data) + 1;
    break;
  case image_INFO_ORDERING:
  case image_INFO_PALETTE:
  default:
    assert(0);
    return result_NOT_IMPLEMENTED;
  }

  if (image_ensure_data(image, nbytes))
    return result_OOM;

  entry->key  = key;
  entry->data = &image->info.data[image->info.dataused];
  memcpy(entry->data, data, nbytes);
  image->info.dataused += nbytes;

  return result_OK;
}

int image_get_info(image_t         *image,
                   image_info_key   key,
                   void           **data)
{
  int i;

  if (image->info.entries == NULL)
    return 0; /* no entries */

  for (i = 0; i < image->info.entriesused; i++)
    if (image->info.entries[i].key == key)
    {
        *data = image->info.entries[i].data;
        return 1;
    }

  return 0;
}

/* ----------------------------------------------------------------------- */

/* Dispose of any transient image data, e.g. calculated histograms. */
static void image_dispose_transient(image_t *i)
{
  /* dispose of histogram data */
  free(i->hists);
  i->hists = NULL;
}

/* ----------------------------------------------------------------------- */

image_t *image_create(void)
{
  image_t *i;

  i = calloc(1, sizeof(*i));
  if (i == NULL)
    return i;

  i->refcount = 1;
  i->background_colour = os_COLOUR_TRANSPARENT;

  image_init_info(i);

  list_add_to_head(&list_anchor, &i->list);

  return i;
}

image_t *image_create_from_file(image_choices *choices,
                          const char          *file_name,
                                bits           file_type,
                                osbool         unsafe)
{
  image_t *i;

  i = image_create();
  if (i == NULL)
    goto Failure;

  i->source.file_type = file_type;

  strcpy(i->file_name, file_name);

  if (loader_export_methods(choices, i, file_type))
    goto Failure;

  if (i->methods.load(choices, i))
    goto Failure;

  if (unsafe)
    i->file_name[0] = '\0';

  return i;


Failure:

  image_destroy(i);

  return NULL;
}

/* ----------------------------------------------------------------------- */

void image_addref(image_t *i)
{
  i->refcount++;
}

void image_deleteref(image_t *i)
{
  i->refcount--;
}

/* ----------------------------------------------------------------------- */

int image_start_editing(image_t *i)
{
  if (i->flags & image_FLAG_EDITING)
    return 1;

  i->flags |= image_FLAG_EDITING;

  return 0;
}

void image_stop_editing(image_t *i)
{
  i->flags &= ~image_FLAG_EDITING;
}

osbool image_is_editing(const image_t *i)
{
  return (i->flags & image_FLAG_EDITING) != 0;
}

/* ----------------------------------------------------------------------- */

void image_preview(image_t *i)
{
  imageobserver_event(i, imageobserver_CHANGE_PREVIEW, NULL);
}

/* ----------------------------------------------------------------------- */

void image_hide(image_t *i)
{
  imageobserver_event(i, imageobserver_CHANGE_HIDDEN, NULL);
}

void image_reveal(image_t *i)
{
  imageobserver_event(i, imageobserver_CHANGE_REVEALED, NULL);
}

/* ----------------------------------------------------------------------- */

void image_focus(image_t *i)
{
  imageobserver_event(i, imageobserver_CHANGE_GAINED_FOCUS, NULL);
}

void image_defocus(image_t *i)
{
  imageobserver_event(i, imageobserver_CHANGE_LOST_FOCUS, NULL);
}

/* ----------------------------------------------------------------------- */

void image_about_to_modify(image_t *i)
{
  imageobserver_event(i, imageobserver_CHANGE_ABOUT_TO_MODIFY, NULL);
}

void image_modified(image_t *i, image_modified_flags flags)
{
  imageobserver_data data;

  i->flags |= image_FLAG_MODIFIED;
  i->last_save_ref = 0;

  image_dispose_transient(i);

  data.modified.flags = flags;

  imageobserver_event(i, imageobserver_CHANGE_MODIFIED, &data);
}

void image_saved(image_t *i)
{
  i->flags &= ~image_FLAG_MODIFIED;

  /* TODO: Enter the world of pain that is implementing support for
   * Message_DataSaved. It's hard to justify the effort since the only other
   * app that supports it that I can find is Edit, and even that is only
   * implementing half the protocol. */

  imageobserver_event(i, imageobserver_CHANGE_SAVED, NULL);
}

/* ----------------------------------------------------------------------- */

void image_destroy(image_t *i)
{
  if (i == NULL)
    return;

  if (--i->refcount <= 0) /* refcount may be zero to start with */
  {
    imageobserver_event(i, imageobserver_CHANGE_ABOUT_TO_DESTROY, NULL);

    list_remove(&list_anchor, &i->list);

    if (i->methods.unload)
      i->methods.unload(i);

    image_destroy_info(i);

    image_dispose_transient(i);

    free(i);
  }
}

/* ----------------------------------------------------------------------- */

void image_select(image_t *i, int index)
{
  /* Only sprites can have their index set. */
  if (i->display.file_type == osfile_TYPE_SPRITE)
    /* FIXME: Bounds check */
    i->details.sprite.index = index;
}

/* ----------------------------------------------------------------------- */

void image_map(image_map_callback *fn, void *opaque)
{
  /* Note that the callback signatures are identical, so we can cast. */

  list_walk(&list_anchor, (list_walk_callback_t *) fn, opaque);
}

/* ----------------------------------------------------------------------- */

static void count_callback(image_t *image, void *opaque)
{
  int *i = opaque;

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

static result_t destroy_metadata_callback(ntree_t *t, void *opaque)
{
  NOT_USED(opaque);

  free(ntree_get_data(t));

  return result_OK;
}

void image_destroy_metadata(ntree_t *metadata)
{
  (void) ntree_walk(metadata,
                    ntree_WALK_POST_ORDER | ntree_WALK_ALL,
                    0,
                    destroy_metadata_callback,
                    NULL);

  ntree_delete(metadata);
}

/* ----------------------------------------------------------------------- */

result_t image_get_digest(image_t *image, unsigned char digest[image_DIGESTSZ])
{
  if ((image->flags & image_FLAG_HAS_DIGEST) == 0)
  {
    md5_from_file(image->file_name, image->digest);

    image->flags |= image_FLAG_HAS_DIGEST;
  }

  memcpy(digest, image->digest, image_DIGESTSZ);

  return result_OK;
}
