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
#include "appengine/datastruct/bitvec.h"
#include "appengine/datastruct/atom.h"
#include "appengine/datastruct/hash.h"
#include "appengine/base/strings.h"
#include "appengine/databases/digest-db.h"

#include "appengine/databases/tag-db.h"

/* ----------------------------------------------------------------------- */

/* Hash bins. */
#define HASHSIZE 97

/* Long enough to hold any identifier. */
#define MAXIDLEN 16

/* Maximum number of tokens we expect on a single line. */
#define MAXTOKENS 64

/* Long enough to hold any database line. */
#define READBUFSZ (MAXIDLEN + MAXTOKENS * 12)

/* Size of buffer used when writing out the database. */
#define WRITEBUFSZ 1024

/* Size of pools used for atom storage. */
#define ATOMBUFSZ 1536

/* Estimated tag length. */
#define ESTATOMLEN 9

/* ----------------------------------------------------------------------- */

static const char signature[] = "1";

/* ----------------------------------------------------------------------- */

static int tagdb_refcount = 0;

error tagdb_init(void)
{
  if (tagdb_refcount++ == 0)
  {
    /* dependencies */

    digestdb_init();
  }

  return error_OK;
}

void tagdb_fin(void)
{
  if (--tagdb_refcount == 0)
  {
    digestdb_fin();
  }
}

/* ----------------------------------------------------------------------- */

/* write out a version numbered header */
static error tagdb_write_header(os_fw f)
{
  static const char comments[] = "# Tags";

  static const struct
  {
    const char *line;
    size_t      length;
  }
  lines[] =
  {
    { comments,  sizeof(comments)  - 1 },
    { signature, sizeof(signature) - 1 },
  };

  int i;

  for (i = 0; i < NELEMS(lines); i++)
  {
    osgbpb_writew(f, (const byte *) lines[i].line, lines[i].length);
    os_bput('\n', f);
  }

  return error_OK;
}

error tagdb_create(const char *filename)
{
  error                  err;
  fileswitch_object_type object_type;

  assert(filename);

  /* write out an empty db, if it doesn't already exist */

  object_type = osfile_read_no_path(filename, NULL, NULL, NULL, NULL);
  if (object_type == fileswitch_NOT_FOUND)
  {
    os_fw f;

    f = osfind_openoutw(osfind_NO_PATH, filename, NULL);
    if (f == 0)
      return error_TAGDB_COULDNT_OPEN_FILE;

    err = tagdb_write_header(f);

    osfind_close(f);

    xosfile_set_type(filename, osfile_TYPE_TEXT);

    if (err)
      goto Failure;
  }

  return error_OK;


Failure:

  return err;
}

void tagdb_delete(const char *filename)
{
  assert(filename);

  xosfile_delete(filename, NULL, NULL, NULL, NULL, NULL);
}

/* ----------------------------------------------------------------------- */

struct tagdb
{
  char                    *filename;

  atom_set_t              *tags; /* indexes tag names */

  struct tagdb_tag_entry *counts;
  int                      c_used;
  int                      c_allocated;

  hash_t                  *hash; /* maps ids to bitvecs holding tag indices */

  struct
  {
    error (*fn)(tagdb *, char *);
  }
  parse;
};

static void destroy_hash_value(void *value)
{
  bitvec_destroy(value);
}

static error tagdb_parse_line(tagdb *db, char *buf)
{
  error          err;
  char          *p;
  int            t;
  const char    *tokens[MAXTOKENS];
  unsigned char  id[digestdb_DIGESTSZ];
  int            i;

  p = buf;

  for (t = 0; t < MAXTOKENS; t++)
  {
    p = strchr(p, ' '); /* split at space */
    if (p == NULL)
      break; /* end of string */

    *p++ = '\0'; /* terminate previous token */

    /* skip any further spaces */
    while (*p == ' ')
      p++;

    if (*p == '\0')
      break; /* hit end of string */

    /* token */
    tokens[t] = p;
  }

  if (t < 1)
    return error_TAGDB_SYNTAX_ERROR; /* no tokens found */

  /* convert ID from ASCII format  hex to binary */
  digestdb_decode(id, buf);

  for (i = 0; i < t; i++)
  {
    tagdb_tag tag;

    err = tagdb_add(db, tokens[i], &tag);
    if (err)
      return err;

    err = tagdb_tagid(db, (char *) id, tag);
    if (err)
      return err;
  }

  return error_OK;
}

static error tagdb_parse_first_line(tagdb *db, char *buf)
{
  /* validate the db signature */

  if (strcmp(buf, signature) != 0)
    return error_TAGDB_INCOMPATIBLE;

  db->parse.fn = tagdb_parse_line;

  return error_OK;
}

static error tagdb_read_db(tagdb *db)
{
  error   err;
  size_t  bufsz;
  char   *buf;
  os_fw   f = 0;
  int     occupied;
  int     used;

  bufsz = READBUFSZ;
  buf = malloc(bufsz);
  if (buf == NULL)
    return error_OOM;

  f = osfind_openinw(osfind_NO_PATH, db->filename, NULL);
  if (f == 0)
    return error_TAGDB_COULDNT_OPEN_FILE;

  db->parse.fn = tagdb_parse_first_line;

  occupied = 0;
  used     = 0;

  for (;;)
  {
    int unread;

    /* try to fill buffer */

    unread = osgbpb_readw(f, (byte *) buf + occupied, bufsz - occupied);
    occupied = bufsz - unread;
    if (occupied == 0)
      break; /* nothing left */

    for (;;)
    {
      char *nl;

      nl = memchr(buf + used, '\n', occupied - used);
      if (!nl)
      {
        if (used == 0)
        {
          /* couldn't find a \n in the whole buffer - and used is 0, so we
           * have the whole buffer in which to look */
          err = error_FILENAMEDB_SYNTAX_ERROR;
          goto Failure;
        }
        else
        {
          /* make space in the buffer, then get it refilled */

          memmove(buf, buf + used, occupied - used);
          occupied -= used;
          used      = 0;

          break; /* need more bytes */
        }
      }

      *nl = '\0'; /* terminate */

      if (buf[used] != '#') /* skip comments */
      {
        err = db->parse.fn(db, buf + used);
        if (err)
          goto Failure;
      }

      used = (nl + 1) - buf;
      if (occupied - used <= 0)
        break;
    }
  }

  err = error_OK;

  /* FALLTHROUGH */

Failure:

  if (f)
    osfind_closew(f);

  free(buf);

  return err;
}

static void no_destroy(void *string) /* FIXME move into hash lib */
{
  NOT_USED(string);
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
                     no_destroy,
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
  err = tagdb_read_db(db);
  if (err)
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

struct commit_state
{
  tagdb  *db;
  char   *buf;
  size_t  bufsz;
  os_fw   f;
};

static int commit_cb(const void *key, const void *value, void *arg)
{
  struct commit_state *state = arg;
  error                err;
  const unsigned char *k;
  const bitvec_t      *v;
  char                 ktext[digestdb_DIGESTSZ * 2 + 1];
  int                  c;
  int                  ntags;
  int                  index;

  k = key;
  v = value;

  digestdb_encode(ktext, k);
  ktext[digestdb_DIGESTSZ * 2] = '\0';

  c = sprintf(state->buf, "%s ", ktext);

  ntags = 0;
  index = -1;
  for (;;)
  {
    index = bitvec_next(v, index);
    if (index < 0)
      break;

    err = tagdb_tagtoname(state->db, index,
                           state->buf + c, state->bufsz - c);
    if (err)
      return -1;

    // should quote any tags containing spaces (or quotes)
    // better if it prepared a list of quoted tags in advance outside of
    // this loop

    c += strlen(state->buf + c);
    state->buf[c++] = ' ';

    ntags++;
  }

  if (ntags > 0)
  {
    state->buf[c - 1] = '\n'; /* overwrite final space */
    osgbpb_writew(state->f, (byte *) state->buf, c);
  }

  return 0;
}

error tagdb_commit(tagdb *db)
{
  error               err;
  struct commit_state state;

  assert(db);

  state.db = db;

  state.buf = malloc(WRITEBUFSZ);
  if (state.buf == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  state.bufsz = WRITEBUFSZ;

  state.f = osfind_openoutw(osfind_NO_PATH, db->filename, NULL);
  if (state.f == 0)
  {
    err = error_TAGDB_COULDNT_OPEN_FILE;
    goto Failure;
  }

  err = tagdb_write_header(state.f);
  if (err)
    goto Failure;

  hash_walk(db->hash, commit_cb, &state);

  err = error_OK;

  /* FALLTHROUGH */

Failure:

  if (state.f)
    osfind_closew(state.f);

  xosfile_set_type(db->filename, osfile_TYPE_TEXT);

  free(state.buf);

  return err;
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

  err = atom_new(db->tags, (const unsigned char *) name, strlen(name) + 1,
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

  return atom_set(db->tags, db->counts[tag].index,
                  (const unsigned char *) name, strlen(name) + 1);
}

error tagdb_enumerate_tags(tagdb *db, int *continuation,
                            tagdb_tag *tag, int *count)
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
    db->counts[tag].count++;

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

  db->counts[tag].count--;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

error tagdb_get_tags_for_id(tagdb *db, const char *id,
                             int *continuation, tagdb_tag *tag)
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

static int getid_cb(const void *key, const void *value, void *arg)
{
  struct enumerate_state *state = arg;

  NOT_USED(value);

  /* work out where we are by counting callbacks (ugh) */
  if (state->count++ < state->start)
    return 0; /* keep going */

  state->found = key;
  return -1; /* stop the walk now */
}

error tagdb_enumerate_ids(tagdb *db,
                           int *continuation,
                           char *buf, size_t bufsz)
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

static int getidbytag_cb(const void *key, const void *value, void *arg)
{
  struct enumerate_state *state = arg;
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

error tagdb_enumerate_ids_by_tag(tagdb *db, tagdb_tag tag,
                                  int *continuation,
                                  char *buf, size_t bufsz)
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

static int getidbytags_cb(const void *key, const void *value, void *arg)
{
  error                   err;
  struct enumerate_state *state = arg;
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

error tagdb_enumerate_ids_by_tags(tagdb *db,
                                   const tagdb_tag *tags, int ntags,
                                   int *continuation,
                                   char *buf, size_t bufsz)
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
