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
#include "appengine/datastruct/dict.h"
#include "appengine/datastruct/hash.h"
#include "appengine/base/strings.h"

#include "appengine/databases/tag-db.h"

/* ----------------------------------------------------------------------- */

/* Hash bins. */
#define HASHSIZE 97

/* Long enough to hold any identifier. */
#define MAXIDLEN 256

/* Maximum number of tokens we expect on a single line. */
#define MAXTOKENS 64

/* Long enough to hold any database line. */
#define READBUFSZ (MAXIDLEN + MAXTOKENS * 12)

/* Size of buffer used when writing out the database. */
#define WRITEBUFSZ 1024

/* ----------------------------------------------------------------------- */

static const char signature[] = "1";

/* ----------------------------------------------------------------------- */

error tagdb__init(void)
{
  return error_OK;
}

void tagdb__fin(void)
{
}

/* ----------------------------------------------------------------------- */

/* write out a version numbered header */
static error tagdb__write_header(os_fw f)
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

error tagdb__create(const char *filename)
{
  error                  err;
  fileswitch_object_type object_type;

  assert(filename);

  /* write out an empty db, if it doesn't already exit */

  object_type = osfile_read_no_path(filename, NULL, NULL, NULL, NULL);
  if (object_type == fileswitch_NOT_FOUND)
  {
    os_fw f;

    f = osfind_openoutw(osfind_NO_PATH, filename, NULL);
    if (f == 0)
      return error_TAGDB_COULDNT_OPEN_FILE;

    err = tagdb__write_header(f);

    osfind_close(f);

    xosfile_set_type(filename, osfile_TYPE_TEXT);

    if (err)
      goto Failure;
  }

  return error_OK;


Failure:

  return err;
}

void tagdb__delete(const char *filename)
{
  assert(filename);

  osfile_delete(filename, NULL, NULL, NULL, NULL);
}

/* ----------------------------------------------------------------------- */

struct tagdb
{
  char                    *filename;

  dict_t                  *ids;
  dict_t                  *tags; /* indexes tag names */

  struct tagdb__tag_entry *counts;
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
  bitvec__destroy(value);
}

static error tagdb__parse_line(tagdb *db, char *buf)
{
  error       err;
  char       *p;
  int         t;
  const char *tokens[MAXTOKENS];
  int         i;

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

  for (i = 0; i < t; i++)
  {
    tagdb__tag tag;

    err = tagdb__add(db, tokens[i], &tag);
    if (err)
      return err;

    err = tagdb__tagid(db, buf, tag);
    if (err)
      return err;
  }

  return error_OK;
}

static error tagdb__parse_first_line(tagdb *db, char *buf)
{
  /* validate the db signature */

  if (strcmp(buf, signature) != 0)
    return error_TAGDB_INCOMPATIBLE;

  db->parse.fn = tagdb__parse_line;

  return error_OK;
}

static error tagdb__read_db(tagdb *db)
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

  db->parse.fn = tagdb__parse_first_line;

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

static void no_destroy(void *string)
{
  NOT_USED(string);
}

error tagdb__open(const char *filename, tagdb **pdb)
{
  error   err;
  char   *filenamecopy = NULL;
  dict_t *ids          = NULL;
  dict_t *tags         = NULL;
  hash_t *hash         = NULL;
  tagdb  *db           = NULL;

  assert(filename);
  assert(pdb);

  filenamecopy = str_dup(filename);
  if (filenamecopy == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  ids = dict__create_tuned(32768 / 33, 32768); /* est. 33 chars/entry */
  if (ids == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  tags = dict__create_tuned(1536 / 170, 1536); /* est. 9 chars/entry */
  if (tags == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  err = hash__create(HASHSIZE,
                     NULL, /* use default hash function */
                     NULL, /* use default string compare */
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
  db->ids         = ids;
  db->tags        = tags;
  db->counts      = NULL;
  db->c_used      = 0;
  db->c_allocated = 0;
  db->hash        = hash;

  /* read the database in */
  err = tagdb__read_db(db);
  if (err)
    goto Failure;

  *pdb = db;

  return error_OK;


Failure:

  free(db);
  hash__destroy(hash);
  dict__destroy(tags);
  dict__destroy(ids);
  free(filenamecopy);

  return err;
}

void tagdb__close(tagdb *db)
{
  if (db == NULL)
    return;

  tagdb__commit(db);

  hash__destroy(db->hash);
  free(db->counts);
  dict__destroy(db->tags);
  dict__destroy(db->ids);

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
  const char          *k;
  const bitvec_t      *v;
  int                  c;
  int                  ntags;
  int                  index;

  k = key;
  v = value;

  c = sprintf(state->buf, "%s ", k);

  ntags = 0;
  index = -1;
  for (;;)
  {
    index = bitvec__next(v, index);
    if (index < 0)
      break;

    err = tagdb__tagtoname(state->db, index,
                           state->buf + c, state->bufsz - c);
    if (err)
      return -1;

    // should quote any tags containing spaces (or quotes)

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

error tagdb__commit(tagdb *db)
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

  err = tagdb__write_header(state.f);
  if (err)
    goto Failure;

  hash__walk(db->hash, commit_cb, &state);

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

typedef struct tagdb__tag_entry
{
  dict_index index;
  int        count;
}
tagdb__tag_entry;

error tagdb__add(tagdb *db, const char *name, tagdb__tag *ptag)
{
  error      err;
  dict_index index;
  tagdb__tag i;

  assert(db);
  assert(name);

  err = dict__add(db->tags, name, &index);
  if (err == error_DICT_NAME_EXISTS)
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

void tagdb__remove(tagdb *db, tagdb__tag tag)
{
  error err;
  int   cont;

  assert(db);
  assert(tag < db->c_used && db->counts[tag].index != -1);

  /* remove this tag from all ids */

  cont = 0;
  do
  {
    char buf[MAXIDLEN];

    err = tagdb__enumerate_ids_by_tag(db, tag, &cont, buf, sizeof(buf));
    if (err)
      goto Failure;

    if (cont)
    {
      err = tagdb__untagid(db, buf, tag);
      if (err)
        goto Failure;
    }
  }
  while (cont);

  /* remove from dictionary - do this after the tag is removed from all ids,
   * so that the tag validity tests don't trigger */

  dict__delete_index(db->tags, db->counts[tag].index);

  db->counts[tag].index = -1;
  db->counts[tag].count = 0;

  err = error_OK;

  /* FALLTHROUGH */

Failure:

  /* the error is absorbed */

  return;
}

error tagdb__rename(tagdb *db, tagdb__tag tag, const char *name)
{
  assert(db);
  assert(tag < db->c_used && db->counts[tag].index != -1);
  assert(name);

  return dict__rename(db->tags, db->counts[tag].index, name);
}

error tagdb__enumerate_tags(tagdb *db, int *continuation,
                            tagdb__tag *tag, int *count)
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

error tagdb__tagtoname(tagdb *db, tagdb__tag tag, char *buf, size_t bufsz)
{
  const char *s;
  size_t      l;

  assert(db);

  if (tag >= db->c_used || db->counts[tag].index == -1)
    return error_TAGDB_UNKNOWN_TAG;

  s = dict__string_and_len(db->tags, &l, db->counts[tag].index);

  if (bufsz < l + 1)
    return error_TAGDB_BUFF_OVERFLOW;

  assert(buf);
  assert(bufsz > 0);

  memcpy(buf, s, l);
  buf[l] = '\0';

  return error_OK;
}

/* ----------------------------------------------------------------------- */

error tagdb__tagid(tagdb *db, const char *id, tagdb__tag tag)
{
  error     err;
  bitvec_t *val;
  int       inc;

  assert(db);
  assert(id);

  if (tag >= db->c_used || db->counts[tag].index == -1)
    return error_TAGDB_UNKNOWN_TAG;

  inc = 1; /* set this if it's a new tagging */

  val = hash__lookup(db->hash, id);
  if (val)
  {
    /* update */

    if (bitvec__get(val, tag)) /* if already set, don't increment counter */
      inc = 0;
    else
      bitvec__set(val, tag);
  }
  else
  {
    dict_index index;

    /* create */

    err = dict__add(db->ids, id, &index);
    if (err == error_DICT_NAME_EXISTS)
      err = error_OK;
    else if (err)
      return err;

    val = bitvec__create(1);
    if (val == NULL)
      return error_OOM;

    bitvec__set(val, tag);

    hash__insert(db->hash, (char *) dict__string(db->ids, index), val);
  }

  if (inc)
    db->counts[tag].count++;

  return error_OK;
}

error tagdb__untagid(tagdb *db, const char *id, tagdb__tag tag)
{
  bitvec_t *val;

  assert(db);
  assert(id);

  if (tag >= db->c_used || db->counts[tag].index == -1)
    return error_TAGDB_UNKNOWN_TAG;

  val = hash__lookup(db->hash, id);
  if (!val)
    return error_TAGDB_UNKNOWN_ID;

  bitvec__clear(val, tag);

  db->counts[tag].count--;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

error tagdb__get_tags_for_id(tagdb *db, const char *id,
                             int *continuation, tagdb__tag *tag)
{
  bitvec_t *v;
  int       index;

  assert(db);
  assert(id);
  assert(continuation);
  assert(tag);

  v = hash__lookup(db->hash, id);
  if (!v)
    return error_TAGDB_UNKNOWN_ID;

  /* To behave like a standard continuation I have to start with zero, but
   * the first value bitvec__next needs to take is -1, so we have to subtract
   * one on entry and add it back on (successful) exit. */

  index = bitvec__next(v, *continuation - 1);

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
  tagdb__tag  tag; /* the tag we want */
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

error tagdb__enumerate_ids(tagdb *db,
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

  if (hash__walk(db->hash, getid_cb, &state) < 0)
  {
    size_t l;

    l = strlen(state.found) + 1;

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

  if (!bitvec__get(v, state->tag))
    return 0; /* keep going */

  state->found = key;
  return -1; /* stop the walk now */
}

error tagdb__enumerate_ids_by_tag(tagdb *db, tagdb__tag tag,
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

  if (hash__walk(db->hash, getidbytag_cb, &state) < 0)
  {
    size_t l;

    l = strlen(state.found) + 1;

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

  err = bitvec__and(state->want, v, &have);
  if (err)
  {
    state->err = err;
    return -1; /* stop the walk now */
  }

  eq = bitvec__eq(have, state->want);

  bitvec__destroy(have);

  if (!eq)
    return 0; /* keep going */

  state->found = key;
  return -1; /* stop the walk now */
}

error tagdb__enumerate_ids_by_tags(tagdb *db,
                                   const tagdb__tag *tags, int ntags,
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

  want = bitvec__create(ntags);
  if (want == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  for (i = 0; i < ntags; i++)
  {
    err = bitvec__set(want, tags[i]);
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

  if (hash__walk(db->hash, getidbytags_cb, &state) < 0)
  {
    size_t l;

    if (state.err)
    {
      err = state.err;
      goto Failure;
    }

    l = strlen(state.found) + 1;

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

  bitvec__destroy(want);

  return err;
}

/* ----------------------------------------------------------------------- */

void tagdb__forget(tagdb *db, const char *id)
{
  assert(db);
  assert(id);

  hash__remove(db->hash, id);
}
