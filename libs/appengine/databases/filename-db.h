/* --------------------------------------------------------------------------
 *    Name: filename-db.h
 * Purpose: Filename database
 * Version: $Id: filename-db.h,v 1.3 2010-01-13 18:41:09 dpt Exp $
 * ----------------------------------------------------------------------- */

// this is probably more general than i've set it up for:
// it could just be 'stringhashdb'

/* The filenamedb maps IDs (MD5 digests) to filenames so that we can search
 * for files and retrieve filenames */

#ifndef APPENGINE_FILENAME_DB_H
#define APPENGINE_FILENAME_DB_H

#include <stddef.h>

#include "appengine/base/errors.h"

#define T filenamedb_t

/* ----------------------------------------------------------------------- */

error filenamedb__init(void);
void filenamedb__fin(void);

/* ----------------------------------------------------------------------- */

error filenamedb__create(const char *filename);
void filenamedb__delete(const char *filename);

/* ----------------------------------------------------------------------- */

typedef struct T T;

error filenamedb__open(const char *filename, T **db);
void filenamedb__close(T *db);

/* force any pending changes to disc */
error filenamedb__commit(T *db);

/* ----------------------------------------------------------------------- */

error filenamedb__add(T          *db,
                      const char *id,
                      const char *filename);

const char *filenamedb__get(T          *db,
                            const char *id);

/* ----------------------------------------------------------------------- */

error filenamedb__prune(T *db);

/* ----------------------------------------------------------------------- */

#undef T

#endif /* APPENGINE_FILENAME_DB_H */
