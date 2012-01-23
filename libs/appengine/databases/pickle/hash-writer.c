/* --------------------------------------------------------------------------
 *    Name: hash-writer.c
 * Purpose: Glue methods to let pickle operate on hashes
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/types.h"

#include "appengine/datastruct/hash.h"

#include "appengine/databases/pickle.h"
#include "appengine/databases/pickle-writer-hash.h"

error pickle_writer_hash_start(void  *assocarr,
                               void **pstate,
                               void  *opaque)
{
  NOT_USED(opaque);

  *pstate = (void *) assocarr;

  return error_OK;
}

error pickle_writer_hash_next(void *state,
                              void *key,
                              void *value,
                              void  *opaque)
{
  NOT_USED(opaque);

  return hash_insert((hash_t *) state, key, value);
}

const pickle_writer_methods pickle_writer_hash =
{
  pickle_writer_hash_start,
  NULL,
  pickle_writer_hash_next
};
