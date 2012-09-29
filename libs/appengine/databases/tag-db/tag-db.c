/* --------------------------------------------------------------------------
 *    Name: tag-db.c
 * Purpose: Tag database
 * ----------------------------------------------------------------------- */

// TODO
// cope with tags with spaces (quoted for saving and loading)
// canonicalise paths - necessary?
// pass in app name for comments

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/osargs.h"
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/osfscontrol.h"
#include "oslib/osgbpb.h"

#include "appengine/types.h"
#include "appengine/base/bitwise.h"
#include "appengine/base/errors.h"
#include "appengine/base/strings.h"
#include "appengine/databases/digest-db.h"
#include "appengine/databases/pickle-reader-hash.h"
#include "appengine/databases/pickle-writer-hash.h"
#include "appengine/databases/pickle.h"
#include "appengine/datastruct/atom.h"
#include "appengine/datastruct/bitvec.h"
#include "appengine/datastruct/hash.h"

#include "appengine/databases/tag-db.h"

/* ----------------------------------------------------------------------- */

/* Hash bins. */
#define HASHSIZE 97

/* Long enough to hold any identifier. */
#define MAXIDLEN 16

/* Maximum number of tokens we expect on a single line. */
#define MAXTOKENS 64

/* Size of pools used for atom storage. */
#define ATOMBUFSZ 1536

/* Estimated tag length. */
#define ESTATOMLEN 9

/* ----------------------------------------------------------------------- */

static unsigned int tagdb_refcount = 0;

error tagdb_init(void)
{
  error err;

  if (tagdb_refcount == 0)
  {
    /* dependencies */

    err = digestdb_init();
    if (err)
      return err;
  }

  tagdb_refcount++;

  return error_OK;
}

void tagdb_fin(void)
{
  if (tagdb_refcount == 0)
    return;

  if (--tagdb_refcount == 0)
  {
    /* dependencies */

    digestdb_fin();
  }
}

/* ----------------------------------------------------------------------- */

struct tagdb
{
  char                   *filename;

  atom_set_t             *tags; /* tag names */

  struct tagdb_tag_entry *counts;
  int                     c_used;
  int                     c_allocated;

  hash_t                 *hash; /* maps ids to bitvecs holding tag indices */
};

static void tagdb__taginc(tagdb *db, tagdb_tag tag);
//static void tagdb__tagdec(tagdb *db, tagdb_tag tag);

/* ----------------------------------------------------------------------- */

static error unformat_key(const char *buf,
                          size_t      len,
                          void      **key,
                          void       *opaque)
{
  error         err;
  unsigned char hash[digestdb_DIGESTSZ];
  int           kindex;

  NOT_USED(len);
  NOT_USED(opaque);

  // if (len != 32) then complain

  /* convert ID from ASCII hex to binary */
  err = digestdb_decode(hash, buf);
  if (err)
    return err;

  err = digestdb_add(hash, &kindex);
  if (err)
    return err;

  *key = (void *) digestdb_get(kindex); /* must cast away const */

  return error_OK;
}

static error unformat_value(const char *inbuf,
                            size_t      len,
                            void      **value,
                            void       *opaque)
{
  error       err;
  char        buf[1024];
  char       *p;
  int         t;
  const char *tokens[MAXTOKENS];
  bitvec_t   *v;
  int         i;
  tagdb      *db = opaque;

  /* copy the buffer so that we can terminate each token as it's found */
  memcpy(buf, inbuf, len);

  p = buf;

  for (t = 0; t < MAXTOKENS;)
  {
    /* skip initial spaces */
    while (*p == ' ')
      p++;

    if (*p == '\0')
      break; /* hit end of string */

    /* token */
    tokens[t++] = p;

    p = strchr(p, ' '); /* split at space */
    if (p == NULL)
      break; /* end of string */

    *p++ = '\0'; /* terminate token */
  }

  if (t < 1)
    return error_TAGDB_SYNTAX_ERROR; /* no tokens found */

  v = bitvec_create(1);
  if (v == NULL)
    return error_OOM;

  for (i = 0; i < t; i++)
  {
    tagdb_tag tag;

    err = tagdb_add(db, tokens[i], &tag);
    if (err)
      return err;

    bitvec_set(v, tag);

    tagdb__taginc(db, tag);
  }

  *value = v;

  return error_OK;
}

static const pickle_unformat_methods unformat_methods =
{
  " ", /* split string */
  1,   /* split string length */
  unformat_key,
  unformat_value
};

/* ----------------------------------------------------------------------- */

static void destroy_hash_value(void *value)
{
  bitvec_destroy(value);
}

error tagdb_open(const char *filename, tagdb **pdb)
{
  error       err;
  char       *filenamecopy = NULL;
  atom_set_t *tags         = NULL;
  hash_t     *hash         = NULL;
  tagdb      *db           = NULL;

  assert(filename);
  assert(pdb);

  filenamecopy = str_dup(filename);
  if (filenamecopy == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  tags = atom_create_tuned(ATOMBUFSZ / ESTATOMLEN, ATOMBUFSZ);
  if (tags == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  err = hash_create(HASHSIZE,
                    digestdb_hash,
                    digestdb_compare,
                    hash_no_destroy_key,
                    destroy_hash_value,
                   &hash);
  if (err)
    goto Failure;

  db = malloc(sizeof(*db));
  if (db == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  db->filename    = filenamecopy;
  db->tags        = tags;
  db->counts      = NULL;
  db->c_used      = 0;
  db->c_allocated = 0;
  db->hash        = hash;

  /* read the database in */
  err = pickle_unpickle(filename,
                        db->hash,
                       &pickle_writer_hash,
                       &unformat_methods,
                        db);
  if (err && err != error_PICKLE_COULDNT_OPEN_FILE)
    goto Failure;

  *pdb = db;

  return error_OK;


Failure:

  free(db);
  hash_destroy(hash);
  atom_destroy(tags);
  free(filenamecopy);

  return err;
}

void tagdb_close(tagdb *db)
{
  if (db == NULL)
    return;

  tagdb_commit(db);

  hash_destroy(db->hash);
  free(db->counts);
  atom_destroy(db->tags);

  free(db->filename);

  free(db);
}

/* ----------------------------------------------------------------------- */

static error format_key(const void *vkey,
                        char       *buf,
                        size_t      len,
                        void       *opaque)
{
  NOT_USED(opaque);

  if (len < digestdb_DIGESTSZ * 2 + 1)
    return error_TAGDB_BUFF_OVERFLOW;

  digestdb_encode(buf, vkey);
  buf[digestdb_DIGESTSZ * 2] = '\0';

  return error_OK;
}

static error format_value(const void *vvalue,
                          char       *buf,
                          size_t      len,
                          void       *opaque)
{
  error           err;
  tagdb          *db = opaque;
  const bitvec_t *v  = vvalue;
  int             c;
  int             index;

  NOT_USED(len);

  c = 0;

  index = -1;
  for (;;)
  {
    index = bitvec_next(v, index);
    if (index < 0)
      break;

    err = tagdb_tagtoname(db, index, buf + c, len - c);
    if (err)
      return -1;

    // should quote any tags containing spaces (or quotes)
    // better if it prepared a list of quoted tags in advance outside of
    // this loop

    c += strlen(buf + c);
    buf[c++] = ' ';
  }

  if (c > 0)
    buf[c - 1] = '\0'; /* overwrite final space */

  /* If no tokens were encoded then return error_PICKLE_SKIP to avoid writing
   * out this empty entry. */
  return (c > 0) ? error_OK : error_PICKLE_SKIP;
}

static const pickle_format_methods format_methods =
{
  "Tags",
  NELEMS("Tags") - 1,
  " ",
  1,
  format_key,
  format_value
};

/* ----------------------------------------------------------------------- */

error tagdb_commit(tagdb *db)
{
  error err;

  err = pickle_pickle(db->filename,
                      db->hash,
                     &pickle_reader_hash,
                     &format_methods,
                      db);
  if (err)
    return err;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

typedef struct tagdb_tag_entry
{
  atom_t index;
  int    count;
}
tagdb_tag_entry;

error tagdb_add(tagdb *db, const char *name, tagdb_tag *ptag)
{
  error      err;
  atom_t     index;
  tagdb_tag i;

  assert(db);
  assert(name);

  err = atom_new(db->tags,
                 (const unsigned char *) name,
                 strlen(name) + 1,
                &index);
  if (err == error_ATOM_NAME_EXISTS)
  {
    /* the name exists in the dict. we have to search to find out which
     * entry we assigned it to. */

    for (i = 0; i < db->c_used; i++)
      if (db->counts[i].index == index)
        break;

    assert(i < db->c_used);

    if (ptag)
      *ptag = i;

    return error_OK;
  }
  else if (err)
    return err;

  /* use up all the entries until we run out of space. when we run out then
   * go hunting for empty entries before extending the block. */

  if (db->c_used >= db->c_allocated) /* out of space? */
  {
    /* search for empty tag entries */
    for (i = 0; i < db->c_used; i++)
      if (db->counts[i].index < 0)
        break;

    if (i == db->c_used)
    {
      /* didn't find any empty entries - have to extend */

      size_t n;
      void  *newarr;

      n = (size_t) power2gt(db->c_allocated);
      if (n < 8)
        n = 8;

      newarr = realloc(db->counts, n * sizeof(*db->counts));
      if (newarr == NULL)
      {
        err = error_OOM;
        goto Failure;
      }

      db->counts      = newarr;
      db->c_allocated = n;

      i = db->c_used++;
    }
    else
    {
      /* found an empty entry */
    }
  }
  else
  {
    i = db->c_used++;
  }

  /* new tag */

  db->counts[i].index = index;
  db->counts[i].count = 0;

  if (ptag)
    *ptag = i;

  return error_OK;


Failure:

  return err;
}

void tagdb_remove(tagdb *db, tagdb_tag tag)
{
  error err;
  int   cont;

  assert(db);
  assert(tag < db->c_used && db->counts[tag].index != -1);

  /* remove this tag from all ids */

  cont = 0;
  do
  {
    char id[MAXIDLEN];

    err = tagdb_enumerate_ids_by_tag(db, tag, &cont, id, sizeof(id));
    if (err)
      goto Failure;

    if (cont)
    {
      err = tagdb_untagid(db, id, tag);
      if (err)
        goto Failure;
    }
  }
  while (cont);

  /* remove from dictionary - do this after the tag is removed from all ids,
   * so that the tag validity tests don't trigger */

  atom_delete(db->tags, db->counts[tag].index);

  db->counts[tag].index = -1;
  db->counts[tag].count = 0;

  err = error_OK;

  /* FALLTHROUGH */

Failure:

  /* the error is absorbed */

  return;
}

error tagdb_rename(tagdb *db, tagdb_tag tag, const char *name)
{
  assert(db);
  assert(tag < db->c_used && db->counts[tag].index != -1);
  assert(name);

  return atom_set(db->tags,
                  db->counts[tag].index,
                  (const unsigned char *) name,
                  strlen(name) + 1);
}

error tagdb_enumerate_tags(tagdb     *db,
                           int       *continuation,
                           tagdb_tag *tag,
                           int       *count)
{
  int index;

  assert(db);
  assert(continuation);
  assert(tag);
  assert(count);

  /* find a used entry */

  for (index = *continuation; index < db->c_used; index++)
    if (db->counts[index].index >= 0)
      break;

  if (index >= db->c_used)
  {
    /* ran out */

    *tag          = 0;
    *count        = 0;
    *continuation = 0;
  }
  else
  {
    /* got one */

    *tag          = db->counts[index].index;
    *count        = db->counts[index].count;
    *continuation = ++index;
  }

  return error_OK;
}

error tagdb_tagtoname(tagdb *db, tagdb_tag tag, char *buf, size_t bufsz)
{
  const char *s;
  size_t      l;

  assert(db);

  if (tag >= db->c_used || db->counts[tag].index == -1)
    return error_TAGDB_UNKNOWN_TAG;

  s = (const char *) atom_get(db->tags, db->counts[tag].index, &l);

  if (bufsz < l)
    return error_TAGDB_BUFF_OVERFLOW;

  assert(buf);
  assert(bufsz > 0);

  memcpy(buf, s, l); /* includes terminator */

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static void tagdb__taginc(tagdb *db, tagdb_tag tag)
{
  db->counts[tag].count++;
}

static void tagdb__tagdec(tagdb *db, tagdb_tag tag)
{
  db->counts[tag].count--;
}

/* This tags and inserts. */
error tagdb_tagid(tagdb *db, const char *id, tagdb_tag tag)
{
  error     err;
  bitvec_t *val;
  int       inc;

  assert(db);
  assert(id);

  if (tag >= db->c_used || db->counts[tag].index == -1)
    return error_TAGDB_UNKNOWN_TAG;

  inc = 1; /* set this if it's a new tagging */

  val = hash_lookup(db->hash, id);
  if (val)
  {
    /* update */

    if (bitvec_get(val, tag)) /* if already set, don't increment counter */
      inc = 0;
    else
      bitvec_set(val, tag);
  }
  else
  {
    int                  kindex;
    const unsigned char *key;

    /* create */

    err = digestdb_add((const unsigned char *) id, &kindex);
    if (err)
      return err;

    key = digestdb_get(kindex);

    val = bitvec_create(1);
    if (val == NULL)
      return error_OOM;

    bitvec_set(val, tag);

    hash_insert(db->hash, (char *) key, val);
  }

  if (inc)
    tagdb__taginc(db, tag);

  return error_OK;
}

error tagdb_untagid(tagdb *db, const char *id, tagdb_tag tag)
{
  bitvec_t *val;

  assert(db);
  assert(id);

  if (tag >= db->c_used || db->counts[tag].index == -1)
    return error_TAGDB_UNKNOWN_TAG;

  val = hash_lookup(db->hash, id);
  if (!val)
    return error_TAGDB_UNKNOWN_ID;

  bitvec_clear(val, tag);

  tagdb__tagdec(db, tag);

  return error_OK;
}

/* ----------------------------------------------------------------------- */

error tagdb_get_tags_for_id(tagdb      *db,
                            const char *id,
                            int        *continuation,
                            tagdb_tag  *tag)
{
  bitvec_t *v;
  int       index;

  assert(db);
  assert(id);
  assert(continuation);
  assert(tag);

  v = hash_lookup(db->hash, id);
  if (!v)
    return error_TAGDB_UNKNOWN_ID;

  /* To behave like a standard continuation I have to start with zero, but
   * the first value bitvec_next needs to take is -1, so we have to subtract
   * one on entry and add it back on (successful) exit. */

  index = bitvec_next(v, *continuation - 1);

  if (index >= 0)
  {
    *tag          = index;
    *continuation = index + 1;
  }
  else
  {
    /* no more tags set */

    *tag          = 0;
    *continuation = 0;
  }

  return error_OK;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* This stuff is grisly. Because we can't remember where we were when
 * terminating a hash walk we need the callback to count callbacks until it
 * can proceed from the original point.
 */

struct enumerate_state
{
  int         start;
  int         count;
  const char *found;
  tagdb_tag  tag; /* the tag we want */
  bitvec_t   *want;
  error       err;
};

static int getid_cb(const void *key, const void *value, void *opaque)
{
  struct enumerate_state *state = opaque;

  NOT_USED(value);

  /* work out where we are by counting callbacks (ugh) */
  if (state->count++ < state->start)
    return 0; /* keep going */

  state->found = key;
  return -1; /* stop the walk now */
}

error tagdb_enumerate_ids(tagdb *db,
                          int   *continuation,
                          char  *buf,
                          size_t bufsz)
{
  struct enumerate_state state;

  assert(db);
  assert(continuation);
  assert(buf);
  assert(bufsz > 0);

  state.start = *continuation;
  state.count = 0;
  state.found = NULL;

  if (hash_walk(db->hash, getid_cb, &state) < 0)
  {
    size_t l;

    l = digestdb_DIGESTSZ;

    if (bufsz < l)
      return error_TAGDB_BUFF_OVERFLOW;

    memcpy(buf, state.found, l);

    *continuation = state.count;
  }
  else
  {
    *continuation = 0;
  }

  return error_OK;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int getidbytag_cb(const void *key, const void *value, void *opaque)
{
  struct enumerate_state *state = opaque;
  const bitvec_t         *v;

  /* work out where we are by counting callbacks (ugh) */
  if (state->count++ < state->start)
    return 0; /* keep going */

  v = value;

  if (!bitvec_get(v, state->tag))
    return 0; /* keep going */

  state->found = key;
  return -1; /* stop the walk now */
}

error tagdb_enumerate_ids_by_tag(tagdb    *db,
                                 tagdb_tag tag,
                                 int      *continuation,
                                 char     *buf,
                                 size_t    bufsz)
{
  struct enumerate_state state;

  assert(db);
  assert(tag < db->c_used && db->counts[tag].index != -1);
  assert(continuation);
  assert(buf);
  assert(bufsz > 0);

  state.start = *continuation;
  state.count = 0;
  state.found = NULL;
  state.tag   = tag;

  if (hash_walk(db->hash, getidbytag_cb, &state) < 0)
  {
    size_t l;

    l = digestdb_DIGESTSZ;

    if (bufsz < l)
      return error_TAGDB_BUFF_OVERFLOW;

    memcpy(buf, state.found, l);

    *continuation = state.count;
  }
  else
  {
    *continuation = 0;
  }

  return error_OK;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int getidbytags_cb(const void *key, const void *value, void *opaque)
{
  error                   err;
  struct enumerate_state *state = opaque;
  const bitvec_t         *v;
  bitvec_t               *have;
  int                     eq;

  /* work out where we are by counting callbacks (ugh) */
  if (state->count++ < state->start)
    return 0; /* keep going */

  /* if ((want & value) == want) then return it; */

  v = value;

  err = bitvec_and(state->want, v, &have);
  if (err)
  {
    state->err = err;
    return -1; /* stop the walk now */
  }

  eq = bitvec_eq(have, state->want);

  bitvec_destroy(have);

  if (!eq)
    return 0; /* keep going */

  state->found = key;
  return -1; /* stop the walk now */
}

error tagdb_enumerate_ids_by_tags(tagdb           *db,
                                  const tagdb_tag *tags,
                                  int              ntags,
                                  int             *continuation,
                                  char            *buf,
                                  size_t           bufsz)
{
  error                  err;
  bitvec_t              *want = NULL;
  int                    i;
  struct enumerate_state state;

  assert(db);
  assert(tags);
  assert(ntags);
  assert(continuation);
  assert(buf);
  assert(bufsz > 0);

  /* form a bitvec of the required tags */

  want = bitvec_create(ntags);
  if (want == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  for (i = 0; i < ntags; i++)
  {
    err = bitvec_set(want, tags[i]);
    if (err)
    {
      goto Failure;
    }
  }

  state.start = *continuation;
  state.count = 0;
  state.found = NULL;
  state.want  = want;
  state.err   = error_OK;

  if (hash_walk(db->hash, getidbytags_cb, &state) < 0)
  {
    size_t l;

    if (state.err)
    {
      err = state.err;
      goto Failure;
    }
    l = digestdb_DIGESTSZ;

    if (bufsz < l)
    {
      err = error_TAGDB_BUFF_OVERFLOW;
      goto Failure;
    }

    memcpy(buf, state.found, l);

    *continuation = state.count;
  }
  else
  {
    *continuation = 0;
  }

  err = error_OK;

  /* FALLTHROUGH */

Failure:

  bitvec_destroy(want);

  return err;
}

/* ----------------------------------------------------------------------- */

void tagdb_forget(tagdb *db, const char *id)
{
  assert(db);
  assert(id);

  hash_remove(db->hash, id);
}
