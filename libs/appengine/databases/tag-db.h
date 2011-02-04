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

error tagdb__init(void);
void tagdb__fin(void);

/* ----------------------------------------------------------------------- */

error tagdb__create(const char *filename);
void tagdb__delete(const char *filename);

/* ----------------------------------------------------------------------- */

typedef struct T T;

error tagdb__open(const char *filename, T **db);
void tagdb__close(T *db);

/* force any pending changes to disc */
error tagdb__commit(T *db);

/* ----------------------------------------------------------------------- */

/* tag management */

typedef unsigned int tagdb__tag;

/* add a new tag */
error tagdb__add(T *db, const char *name, tagdb__tag *tag);

/* delete a tag */
void tagdb__remove(T *db, tagdb__tag tag);

/* rename a tag */
error tagdb__rename(T *db, tagdb__tag tag, const char *name);

/* enumerate tags with counts */
error tagdb__enumerate_tags(T *db, int *continuation,
                            tagdb__tag *tag, int *count);

/* convert a tag to a name */
/* 'buf' may be NULL if bufsz is 0 */
error tagdb__tagtoname(T *db, tagdb__tag tag, char *buf, size_t bufsz);

/* ----------------------------------------------------------------------- */

/* tagging */

/* apply tag to id */
error tagdb__tagid(T *db, const char *id, tagdb__tag tag);

/* remove tag from id */
error tagdb__untagid(T *db, const char *id, tagdb__tag tag);

/* ----------------------------------------------------------------------- */

/* queries */

/* query tags for id */
error tagdb__get_tags_for_id(T *db, const char *id,
                             int *continuation, tagdb__tag *tag);

/* enumerate ids */
error tagdb__enumerate_ids(T *db,
                           int *continuation,
                           char *buf, size_t bufsz);

/* enumerate ids by tag */
error tagdb__enumerate_ids_by_tag(T *db, tagdb__tag tag,
                                  int *continuation,
                                  char *buf, size_t bufsz);

/* enumerate ids which match all specified tags */
error tagdb__enumerate_ids_by_tags(T *db,
                                   const tagdb__tag *tags, int ntags,
                                   int *continuation,
                                   char *buf, size_t bufsz);

/* ----------------------------------------------------------------------- */

/* delete knowledge of id */
void tagdb__forget(T *db, const char *id);

/* ----------------------------------------------------------------------- */

#undef T

#endif /* APPENGINE_TAG_DB_H */
