/* --------------------------------------------------------------------------
 *    Name: tag-cloud.h
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TAG_CLOUD_H
#define APPENGINE_TAG_CLOUD_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"

#define T tag_cloud

/* ----------------------------------------------------------------------- */

typedef struct T T;

/* ----------------------------------------------------------------------- */

error tag_cloud__init(void);

void tag_cloud__fin(void);

/* ----------------------------------------------------------------------- */

typedef struct tag_cloud__config
{
  int size;    /* points (-1 to use desktop font size) */
  int scale;   /* percent */
  int leading; /* 24.8 fixed */
  int padding; /* OS units */
}
tag_cloud__config;

/* ----------------------------------------------------------------------- */

/* the toolbar is normally visible by default - this hides it */
#define tag_cloud__CREATE_FLAG_TOOLBAR_HIDDEN   (1u << 0)
/* this disallows the toolbar entirely */
#define tag_cloud__CREATE_FLAG_TOOLBAR_DISABLED (1u << 1)

typedef unsigned int tag_cloud__create_flags;

T *tag_cloud__create(tag_cloud__create_flags  flags,
                     const tag_cloud__config *config);

void tag_cloud__destroy(T *doomed);

/* ----------------------------------------------------------------------- */

error tag_cloud__set_config(T                       *tc,
                            const tag_cloud__config *config);

/* ----------------------------------------------------------------------- */

typedef enum tag_cloud__event
{
  tag_cloud__EVENT_CLOSE,

  tag_cloud__EVENT_DISPLAY_LIST,
  tag_cloud__EVENT_DISPLAY_CLOUD,

  tag_cloud__EVENT_SORT_BY_NAME,
  tag_cloud__EVENT_SORT_BY_COUNT,

  tag_cloud__EVENT_SORT_SELECTED_FIRST,

  tag_cloud__EVENT_INFO,
  tag_cloud__EVENT_KILL,
  tag_cloud__EVENT_NEW,
  tag_cloud__EVENT_RENAME,

  tag_cloud__EVENT_COMMIT,

  tag_cloud__EVENT_UNKNOWN,
}
tag_cloud__event;

/* ----------------------------------------------------------------------- */

/* Create a new tag. */
// must client call settags again?
typedef error (tag_cloud__newtagfn)(T          *tc,
                                    const char *name,
                                    int         length,
                                    void       *arg);

/* Delete 'index'. */
typedef error (tag_cloud__deletetagfn)(T    *tc,
                                       int   index,
                                       void *arg);

/* Rename 'index' to 'name'. */
typedef error (tag_cloud__renametagfn)(T          *tc,
                                       int         index,
                                       const char *name,
                                       int         length,
                                       void       *arg);

/* Tag or detag the current file with 'index'.
 * Called when the user clicks on a tag in the the window. */
typedef error (tag_cloud__tagfn)(T    *tc,
                                 int   index,
                                 void *arg);

/* Tag or detag the file 'filename' with 'index'.
 * Called when files are dropped in the window. */
typedef error (tag_cloud__tagfilefn)(T  *tc,
                                     const char *filename,
                                     int         index,
                                     void       *arg);

/* Some other sort of event. */
typedef error (tag_cloud__eventfn)(T                *tc,
                                   tag_cloud__event  event,
                                   void             *arg);

void tag_cloud__set_handlers(T                      *tc,
                             tag_cloud__newtagfn    *newtag,
                             tag_cloud__deletetagfn *deletetag,
                             tag_cloud__renametagfn *renametag,
                             tag_cloud__tagfn       *tag,
                             tag_cloud__tagfn       *detag,
                             tag_cloud__tagfilefn   *tagfile,
                             tag_cloud__tagfilefn   *detagfile,
                             tag_cloud__eventfn     *event,
                             void                   *arg);

/* ----------------------------------------------------------------------- */

typedef struct tag_cloud__tag
{
  const char *name;
  int         count;
}
tag_cloud__tag;

error tag_cloud__set_tags(T                    *tc,
                          const tag_cloud__tag *tags,
                          int                   ntags);

/* ----------------------------------------------------------------------- */

/* Highlights the specified indices.
 * Indices must be given in ascending order.
 * Use nindices == 0 and indices == NULL to clear highlights. */
error tag_cloud__highlight(T *tc, const int *indices, int nindices);

error tag_cloud__add_highlight(T *tc, int index);
void tag_cloud__remove_highlight(T *tc, int index);

/* ----------------------------------------------------------------------- */

#define tag_cloud__DISPLAY_TYPE_LIST   0
#define tag_cloud__DISPLAY_TYPE_CLOUD  1
#define tag_cloud__DISPLAY_TYPE__LIMIT 2

void tag_cloud__set_display(T *tc, int display_type);

int tag_cloud__get_display(T *tc);

/* ----------------------------------------------------------------------- */

#define tag_cloud__SORT_TYPE_NAME   0
#define tag_cloud__SORT_TYPE_COUNT  1
#define tag_cloud__SORT_TYPE__LIMIT 2

void tag_cloud__set_sort(T *tc, int sort_type);

int tag_cloud__get_sort(T *tc);

/* ----------------------------------------------------------------------- */

#define tag_cloud__ORDER_NONE           0
#define tag_cloud__ORDER_SELECTED_FIRST 1
#define tag_cloud__ORDER__LIMIT         2

void tag_cloud__set_order(T *tc, int order_type);

void tag_cloud__toggle_order(T *tc);

int tag_cloud__get_order(T *tc);

/* ----------------------------------------------------------------------- */

/* Dim the window contents and inhibit any activity.
 * Used when no taggable entity is present. */
void tag_cloud__shade(T *tc, int shade);

/* ----------------------------------------------------------------------- */

wimp_w tag_cloud__get_window_handle(T *tc);

/* ----------------------------------------------------------------------- */

/* Tag Cloud asks what to do with the specified key.
 * Return the event to perform. Return -1 if you don't know. */
typedef tag_cloud__event (tag_cloud__key_handler_fn)(wimp_key_no  key_no,
                                                     void        *arg);

void tag_cloud__set_key_handler(T                         *tc,
                                tag_cloud__key_handler_fn *key,
                                void                      *arg);

/* ----------------------------------------------------------------------- */

void tag_cloud__open(T *tc);

#undef T

#endif /* APPENGINE_TAG_CLOUD_H */
