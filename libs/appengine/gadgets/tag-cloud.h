/* --------------------------------------------------------------------------
 *    Name: tag-cloud.h
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TAG_CLOUD_H
#define APPENGINE_TAG_CLOUD_H

#include <stddef.h>

#include "oslib/wimp.h"

#include "appengine/base/errors.h"

#define T tag_cloud

/* ----------------------------------------------------------------------- */

typedef struct T T;

/* ----------------------------------------------------------------------- */

error tag_cloud_init(void);

void tag_cloud_fin(void);

/* ----------------------------------------------------------------------- */

/**
 * Configuration of a tag cloud.
 */
typedef struct tag_cloud_config
{
  int size;    /**< base font size in whole points, e.g. 12,
                    or -1 to use the desktop font size */
  int scale;   /**< overall scale factor, in percent */
  int leading; /**< inter-line spacing, 24.8 fixed scale factor
                    (256 => leading equals font size) */
  int padding; /**< amount of padding to put around the edge of the tag
                    cloud, in OS units */
}
tag_cloud_config;

/* ----------------------------------------------------------------------- */

/**
 * Flags specified to tag_cloud_create.
 */
typedef unsigned int tag_cloud_create_flags;

/**
 * The toolbar is normally visible by default - this hides it.
 */
#define tag_cloud_CREATE_FLAG_TOOLBAR_HIDDEN   (1u << 0)

/**
 * Disallow the toolbar entirely.
 */
#define tag_cloud_CREATE_FLAG_TOOLBAR_DISABLED (1u << 1)

T *tag_cloud_create(tag_cloud_create_flags  flags,
                    const tag_cloud_config *config);

void tag_cloud_destroy(T *doomed);

/* ----------------------------------------------------------------------- */

error tag_cloud_set_config(T                      *tc,
                           const tag_cloud_config *config);

/* ----------------------------------------------------------------------- */

/**
 * Events delivered from, or to, the tag cloud.
 */
typedef enum tag_cloud_event
{
  tag_cloud_EVENT_CLOSE,

  tag_cloud_EVENT_DISPLAY_LIST,
  tag_cloud_EVENT_DISPLAY_CLOUD,

  tag_cloud_EVENT_SORT_BY_NAME,
  tag_cloud_EVENT_SORT_BY_COUNT,

  tag_cloud_EVENT_SORT_SELECTED_FIRST, /* note: this toggles the state */

  tag_cloud_EVENT_INFO,
  tag_cloud_EVENT_KILL,
  tag_cloud_EVENT_NEW,
  tag_cloud_EVENT_RENAME,

  tag_cloud_EVENT_COMMIT,

  tag_cloud_EVENT_UNKNOWN,
}
tag_cloud_event;

/* ----------------------------------------------------------------------- */

// TODO: Document whether it's expected that after these callbacks that the
// client call settags again.

/**
 * Callback to create a new tag.
 */
typedef error (tag_cloud_newtagfn)(T          *tc,
                                   const char *name,
                                   int         length,
                                   void       *opaque);

/**
 * Callback to delete tag 'index'.
 */
typedef error (tag_cloud_deletetagfn)(T    *tc,
                                      int   index,
                                      void *opaque);

/**
 * Callback to rename tag 'index' to 'name'.
 */
typedef error (tag_cloud_renametagfn)(T          *tc,
                                      int         index,
                                      const char *name,
                                      int         length,
                                      void       *opaque);

/**
 * Callback to tag or detag the current file with 'index'.
 * Called when the user clicks on a tag in the the tag cloud.
 */
typedef error (tag_cloud_tagfn)(T    *tc,
                                int   index,
                                void *opaque);

/**
 * Callback to tag or detag the file 'filename' with 'index'.
 * Called when files are dropped in the window.
 */
typedef error (tag_cloud_tagfilefn)(T          *tc,
                                    const char *filename,
                                    int         index,
                                    void       *opaque);

/**
 * Callback for other sorts of event.
 */
typedef error (tag_cloud_eventfn)(T                *tc,
                                  tag_cloud_event  event,
                                  void             *opaque);

/**
 * Set event handlers.
 *
 * \param tc     Tag cloud.
 * \param opaque Opaque value passed to callbacks.
 */
void tag_cloud_set_handlers(T                     *tc,
                            tag_cloud_newtagfn    *newtag,
                            tag_cloud_deletetagfn *deletetag,
                            tag_cloud_renametagfn *renametag,
                            tag_cloud_tagfn       *tag,
                            tag_cloud_tagfn       *detag,
                            tag_cloud_tagfilefn   *tagfile,
                            tag_cloud_tagfilefn   *detagfile,
                            tag_cloud_eventfn     *event,
                            void                  *opaque);

/* ----------------------------------------------------------------------- */

/**
 * Specifies a tag name along with its count.
 */
typedef struct tag_cloud_tag
{
  const char *name;   /**< tag name, need not be terminated */
  int         count;  /**< number of times used */
}
tag_cloud_tag;

/**
 * Populate a tag cloud with tags.
 *
 * When tags are quoted in callbacks and highlights the i'th tag in the input
 * data will have index i.
 *
 * \param tc    Tag cloud.
 * \param tags  Array of tag_cloud_tag values to set.
 * \param ntags Number of values given.
 *
 * \return Error indication.
 */
error tag_cloud_set_tags(T                   *tc,
                         const tag_cloud_tag *tags,
                         int                  ntags);

/* ----------------------------------------------------------------------- */

/**
 * Highlight all of the specified indices.
 *
 * \param tc       Tag cloud.
 * \param indices  Array of indices to highlight.
 * \param nindices Number of indices given, or zero to clear all highlights.
 *
 * \return Error indication.
 */
error tag_cloud_highlight(T *tc, const int *indices, int nindices);

/**
 * Add a single highlight.
 */
error tag_cloud_add_highlight(T *tc, int index);

/**
 * Remove a single highlight.
 */
void tag_cloud_remove_highlight(T *tc, int index);

/* ----------------------------------------------------------------------- */

#define tag_cloud_DISPLAY_TYPE_LIST   0
#define tag_cloud_DISPLAY_TYPE_CLOUD  1
#define tag_cloud_DISPLAY_TYPE__LIMIT 2

void tag_cloud_set_display(T *tc, int display_type);

int tag_cloud_get_display(T *tc);

/* ----------------------------------------------------------------------- */

#define tag_cloud_SORT_TYPE_NAME   0
#define tag_cloud_SORT_TYPE_COUNT  1
#define tag_cloud_SORT_TYPE__LIMIT 2

void tag_cloud_set_sort(T *tc, int sort_type);

int tag_cloud_get_sort(T *tc);

/* ----------------------------------------------------------------------- */

#define tag_cloud_ORDER_NONE           0
#define tag_cloud_ORDER_SELECTED_FIRST 1
#define tag_cloud_ORDER__LIMIT         2

void tag_cloud_set_order(T *tc, int order_type);

void tag_cloud_toggle_order(T *tc);

int tag_cloud_get_order(T *tc);

/* ----------------------------------------------------------------------- */

/**
 * Dim the window contents and inhibit any tagging.
 *
 * \param tc    Tag cloud.
 * \param shade On or off.
 */
void tag_cloud_shade(T *tc, int shade);

/* ----------------------------------------------------------------------- */

wimp_w tag_cloud_get_window_handle(T *tc);

/* ----------------------------------------------------------------------- */

// FIXME: Why is this section down here, and not nearer to the other event
// handler definitions? Why is tag_cloud_set_key_handler separate from
// tag_cloud_set_handlers?

/**
 * Callback which asks what to do with the specified key.
 *
 * Return the tag_cloud_event to perform or -1 if you don't know.
 */
typedef tag_cloud_event (tag_cloud_key_handler_fn)(wimp_key_no key_no,
                                                   void       *opaque);

/**
 * Set key handler.
 *
 * \param tc     Tag cloud.
 * \param key    Key handler function.
 * \param opaque Opaque value passed to callback.
 */
void tag_cloud_set_key_handler(T                        *tc,
                               tag_cloud_key_handler_fn *key,
                               void                     *opaque);

/* ----------------------------------------------------------------------- */

void tag_cloud_open(T *tc);

/* ----------------------------------------------------------------------- */

#undef T

#endif /* APPENGINE_TAG_CLOUD_H */
