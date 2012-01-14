/* --------------------------------------------------------------------------
 *    Name: tag-db.h
 * Purpose: Tag database
 * ----------------------------------------------------------------------- */

/* The tagdb maintains a series of key-value pairs. The key is a string (a
 * filename, or a hash), the value is a set of tags. */

/* Continuations
 * Set continuation to zero to begin with, it'll return zero when no more
 * tags are available. */

#ifndef APPENGINE_TAG_DB_H
#define APPENGINE_TAG_DB_H

#include <stddef.h>

#include "appengine/base/errors.h"

#define T tagdb

/* ----------------------------------------------------------------------- */

error tagdb_init(void);
void tagdb_fin(void);

/* ----------------------------------------------------------------------- */

error tagdb_create(const char *filename);
void tagdb_delete(const char *filename);

/* ----------------------------------------------------------------------- */

typedef struct T T;

error tagdb_open(const char *filename, T **db);
void tagdb_close(T *db);

/* force any pending changes to disc */
error tagdb_commit(T *db);

/* ----------------------------------------------------------------------- */

/* tag management */

typedef unsigned int tagdb_tag;

/* add a new tag */
error tagdb_add(T *db, const char *name, tagdb_tag *tag);

/* delete a tag */
void tagdb_remove(T *db, tagdb_tag tag);

/* rename a tag */
error tagdb_rename(T *db, tagdb_tag tag, const char *name);

/* enumerate tags with counts */
error tagdb_enumerate_tags(T         *db,
                           int       *continuation,
                           tagdb_tag *tag,
                           int       *count);

/* convert a tag to a name */
/* 'buf' may be NULL if bufsz is 0 */
error tagdb_tagtoname(T *db, tagdb_tag tag, char *buf, size_t bufsz);

/* ----------------------------------------------------------------------- */

/* tagging */

/* apply tag to id */
error tagdb_tagid(T *db, const char *id, tagdb_tag tag);

/* remove tag from id */
error tagdb_untagid(T *db, const char *id, tagdb_tag tag);

/* ----------------------------------------------------------------------- */

/* queries */

/* query tags for id */
error tagdb_get_tags_for_id(T          *db,
                            const char *id,
                            int        *continuation,
                            tagdb_tag  *tag);

/* enumerate ids */
error tagdb_enumerate_ids(T     *db,
                          int   *continuation,
                          char  *buf,
                          size_t bufsz);

/* enumerate ids by tag */
error tagdb_enumerate_ids_by_tag(T        *db,
                                 tagdb_tag tag,
                                 int      *continuation,
                                 char     *buf,
                                 size_t    bufsz);

/* enumerate ids which match all specified tags */
error tagdb_enumerate_ids_by_tags(T               *db,
                                  const tagdb_tag *tags,
                                  int              ntags,
                                  int             *continuation,
                                  char            *buf,
                                  size_t           bufsz);

/* ----------------------------------------------------------------------- */

/* delete knowledge of id */
void tagdb_forget(T *db, const char *id);

/* ----------------------------------------------------------------------- */

#undef T

#endif /* APPENGINE_TAG_DB_H */
