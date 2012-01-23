/* --------------------------------------------------------------------------
 *    Name: digest-db.h
 * Purpose: Digest database
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_DIGEST_DB_H
#define APPENGINE_DIGEST_DB_H

#include <stddef.h>

#include "appengine/base/errors.h"

/* ----------------------------------------------------------------------- */

#define digestdb_DIGESTSZ 16

/* ----------------------------------------------------------------------- */

error digestdb_init(void);
void digestdb_fin(void);

/* ----------------------------------------------------------------------- */

error digestdb_add(const unsigned char *digest, int *index);
const unsigned char *digestdb_get(int index);

/* ----------------------------------------------------------------------- */

unsigned int digestdb_hash(const void *a);
int digestdb_compare(const void *a, const void *b);

/* ----------------------------------------------------------------------- */

/* Decode 32 characters of ASCII hex to 16 bytes.
 * Returns error_BAD_ARG if nonhex data is encountered. */
error digestdb_decode(unsigned char *digest, const char *text);

/* Encode 16 bytes to 32 characters of ASCII hex. */
void digestdb_encode(char *text, const unsigned char *digest);

/* ----------------------------------------------------------------------- */

#endif /* APPENGINE_DIGEST_DB_H */
