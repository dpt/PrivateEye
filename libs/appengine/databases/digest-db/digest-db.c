/* --------------------------------------------------------------------------
 *    Name: digest-db.c
 * Purpose: Digest database
 * ----------------------------------------------------------------------- */

#include <stddef.h>
#include <string.h>

#include "appengine/base/errors.h"

#include "appengine/datastruct/atom.h"

#include "appengine/databases/digest-db.h"

/* ----------------------------------------------------------------------- */

#define DIGESTSZ 16

/* ----------------------------------------------------------------------- */

/* We use an atom set as the store at the moment. This is a bit wasteful as
 * a length field is stored per entry and it's always 16 bytes. */

static struct
{
  static atom_set_t *digests;
}
LOCALS;

/* ----------------------------------------------------------------------- */

static int digestdb__refcount = 0;

error digestdb_init(void)
{
  if (digestdb__refcount++ == 0)
  {
    /* init */

    LOCALS.digests = atom_create_tuned(32768 / DIGESTSZ, 32768);
    if (LOCALS.digests == NULL)
      return error_OOM;
  }

  return error_OK;
}

void digestdb_fin(void)
{
  if (--digestdb__refcount == 0)
  {
    atom_destroy(LOCALS.digests);
  }
}

/* ----------------------------------------------------------------------- */

error digestdb_add(const unsigned char *digest, int *index)
{
  error err;

  err = atom_new(LOCALS.digests, digest, DIGESTSZ, (atom_t *) index);
  if (err == error_ATOM_NAME_EXISTS)
    err = error_OK;

  return err;
}

const unsigned char *digestdb_get(int index)
{
  return atom_get(LOCALS.digests, index, NULL);
}

/* ----------------------------------------------------------------------- */

unsigned int digestdb_hash(const void *a)
{
  const unsigned char *ma = a;

  /* it's already a hash: just mask off enough bits */

  return *ma; /* assumes <= 256 hash bins */
}

int digestdb_compare(const void *a, const void *b)
{
  const unsigned char *ma = a;
  const unsigned char *mb = b;

  return memcmp(ma, mb, DIGESTSZ);
}

/* ----------------------------------------------------------------------- */

/* FIXME No input validation. */
void digestdb_decode(unsigned char *bytes, const char *text)
{
#define _ -1

  static const char tab[] =
  {
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, _, _, _, _, _, _,
    _,10,11,12,13,14,15, _, _, _, _, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    _,10,11,12,13,14,15, _, _, _, _, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
    _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,
  };

  const char *end;

  for (end = text + 32; text < end; text += 2)
    *bytes++ = (tab[text[0]] << 4) | tab[text[1]];

#undef _
}

void digestdb_encode(char *text, const unsigned char *bytes)
{
  static const char tab[] = "0123456789abcdef";

  const unsigned char *end;

  for (end = bytes + 16; bytes < end; bytes++)
  {
    unsigned char b;

    b = *bytes;

    *text++ = tab[(b & 0xf0) >> 4];
    *text++ = tab[(b & 0x0f) >> 0];
  }
}

/* ----------------------------------------------------------------------- */

