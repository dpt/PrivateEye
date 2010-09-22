/* --------------------------------------------------------------------------
 *    Name: tags-common.h
 * Purpose: Common tags behaviour
 * Version: $Id: tags-common.h,v 1.7 2010-01-29 15:09:00 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef TAGS_COMMON_H
#define TAGS_COMMON_H

#include "appengine/app/choices.h"
#include "appengine/base/errors.h"
#include "appengine/databases/filename-db.h"
#include "appengine/gadgets/tag-cloud.h"
#include "appengine/graphics/image.h"

tagdb *tags_common__get_db(void);

filenamedb_t *tags_common__get_filename_db(void);


void tags_common__choices_updated(const choices *cs);


/* The following assume that 'arg' is the tagdb pointer. */

error tags_common__add_tag(tag_cloud  *tc,
                           const char *name,
                           int         length,
                           void       *arg);

/* Delete 'index'. */
error tags_common__delete_tag(tag_cloud *tc,
                              int        index,
                              void      *arg);

/* Rename 'index' to 'name'. */
error tags_common__rename_tag(tag_cloud  *tc,
                              int         index,
                              const char *name,
                              int         length,
                              void       *arg);

error tags_common__tag(tag_cloud  *tc,
                       int         index,
                       const char *digest,
                       const char *file_name,
                       void       *arg);

error tags_common__detag(tag_cloud  *tc,
                         int         index,
                         const char *digest,
                         void       *arg);

error tags_common__tagfile(tag_cloud  *tc,
                           const char *file_name,
                           int         index,
                           void       *arg);

error tags_common__detagfile(tag_cloud  *tc,
                             const char *file_name,
                             int         index,
                             void       *arg);

error tags_common__event(tag_cloud        *tc,
                         tag_cloud__event  event,
                         void             *arg);

tag_cloud__event tags_common__keyhandler(wimp_key_no  key_no,
                                         void        *arg);


error tags_common__set_tags(tag_cloud *tc);
error tags_common__set_highlights(tag_cloud *tc, image *image);
error tags_common__clear_highlights(tag_cloud *tc);


error tags_common__init(void);
void tags_common__fin(void);


error tags_common__properinit(void);
void tags_common__properfin(int force);


#endif /* TAGS_COMMON_H */
