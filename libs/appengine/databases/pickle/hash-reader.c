/* --------------------------------------------------------------------------
 *    Name: hash-reader.c
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
#include "appengine/databases/pickle-reader-hash.h"

typedef struct mystate
{
  hash_t *hash;
  int     cont;
}
mystate;

error pickle_reader_hash_start(const void *assocarr,
                               void       *opaque,
                               void      **pstate)
{
  mystate *state;

  NOT_USED(opaque);

  state = malloc(sizeof(*state));
  if (state == NULL)
    return NULL;

  state->hash = (hash_t *) assocarr;
  state->cont = 0;

  *pstate = state;

  return error_OK;
}

void pickle_reader_hash_stop(void *state, void *opaque)
{
  NOT_USED(opaque);

  free(state);
}

error pickle_reader_hash_next(void        *vstate,
                              const void **key,
                              const void **value,
                              void        *opaque)
{
  error    err;
  mystate *state = vstate;

  NOT_USED(opaque);

  err = hash_walk_continuation(state->hash,
                               state->cont,
                              &state->cont,
                               key,
                               value);

  return (err == error_HASH_END) ? error_PICKLE_END : err;
}

const pickle_reader_methods pickle_reader_hash =
{
  pickle_reader_hash_start,
  pickle_reader_hash_stop,
  pickle_reader_hash_next
};
