/* --------------------------------------------------------------------------
 *    Name: filename-db.h
 * Purpose: Filename database
 * ----------------------------------------------------------------------- */

// this is probably more general than i've set it up for:
// it could just be 'stringhashdb'

/* The filenamedb maps IDs (MD5 digests) to filenames so that we can search
 * for files and retrieve filenames */

#ifndef APPENGINE_FILENAME_DB_H
#define APPENGINE_FILENAME_DB_H

#include <stddef.h>

#include "appengine/base/errors.h"
#include "appengine/databases/pickle.h"

#define T filenamedb_t

/* ----------------------------------------------------------------------- */

error filenamedb_init(void);
void filenamedb_fin(void);

/* ----------------------------------------------------------------------- */

#define filenamedb_delete pickle_delete

/* ----------------------------------------------------------------------- */

typedef struct T T;

error filenamedb_open(const char *filename, T **db);
void filenamedb_close(T *db);

/* force any pending changes to disc */
error filenamedb_commit(T *db);

/* ----------------------------------------------------------------------- */

error filenamedb_add(T          *db,
                     const char *id,
                     const char *filename);

const char *filenamedb_get(T          *db,
                           const char *id);

/* ----------------------------------------------------------------------- */

error filenamedb_prune(T *db);

/* ----------------------------------------------------------------------- */

#undef T

#endif /* APPENGINE_FILENAME_DB_H */
