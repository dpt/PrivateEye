/* --------------------------------------------------------------------------
 *    Name: tags-common.h
 * Purpose: Common tags behaviour
 * ----------------------------------------------------------------------- */

#ifndef TAGS_COMMON_H
#define TAGS_COMMON_H

#include "appengine/app/choices.h"
#include "appengine/base/errors.h"
#include "appengine/databases/filename-db.h"
#include "appengine/gadgets/tag-cloud.h"
#include "appengine/graphics/image.h"

tagdb *tags_common_get_db(void);

filenamedb_t *tags_common_get_filename_db(void);


void tags_common_choices_updated(const choices *cs);


/* The following assume that 'opaque' is the tagdb pointer. */

error tags_common_add_tag(tag_cloud  *tc,
                           const char *name,
                           int         length,
                           void       *opaque);

/* Delete 'index'. */
error tags_common_delete_tag(tag_cloud *tc,
                              int        index,
                              void      *opaque);

/* Rename 'index' to 'name'. */
error tags_common_rename_tag(tag_cloud  *tc,
                              int         index,
                              const char *name,
                              int         length,
                              void       *opaque);

error tags_common_tag(tag_cloud  *tc,
                       int         index,
                       const char *digest,
                       const char *file_name,
                       void       *opaque);

error tags_common_detag(tag_cloud  *tc,
                         int         index,
                         const char *digest,
                         void       *opaque);

error tags_common_tagfile(tag_cloud  *tc,
                           const char *file_name,
                           int         index,
                           void       *opaque);

error tags_common_detagfile(tag_cloud  *tc,
                             const char *file_name,
                             int         index,
                             void       *opaque);

error tags_common_event(tag_cloud       *tc,
                         tag_cloud_event  event,
                         void            *opaque);

tag_cloud_event tags_common_keyhandler(wimp_key_no  key_no,
                                         void       *opaque);


error tags_common_set_tags(tag_cloud *tc);
error tags_common_set_highlights(tag_cloud *tc, image_t *image);
error tags_common_clear_highlights(tag_cloud *tc);


error tags_common_init(void);
void tags_common_fin(void);


error tags_common_lazyinit(void);
void tags_common_lazyfin(int force);


#endif /* TAGS_COMMON_H */
