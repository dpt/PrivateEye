/* --------------------------------------------------------------------------
 *    Name: filename-db.c
 * Purpose: Filename database
 * ----------------------------------------------------------------------- */

/* filenamedb maps md5 digests to filenames so that we can search for files
 * and retrieve filenames */

// this should be refactored into a 'serialisable hash' base class

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

//#include "oslib/osargs.h"
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/osfscontrol.h"
#include "oslib/osgbpb.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/datastruct/atom.h"
#include "appengine/datastruct/hash.h"
#include "appengine/base/strings.h"
#include "appengine/databases/digest-db.h"

#include "appengine/databases/filename-db.h"

/* ----------------------------------------------------------------------- */

/* Hash bins. */
#define HASHSIZE 97

/* Long enough to hold any database line. */
#define READBUFSZ 1024

/* Size of buffer used when writing out the database. */
#define WRITEBUFSZ 1024

/* ----------------------------------------------------------------------- */

static const char signature[] = "1";

/* ----------------------------------------------------------------------- */

static int filenamedb__refcount = 0;

error filenamedb__init(void)
{
  if (filenamedb__refcount++ == 0)
  {
    /* dependencies */

    digestdb_init();
  }

  return error_OK;
}

void filenamedb__fin(void)
{
  if (--filenamedb__refcount == 0)
  {
    digestdb_fin();
  }
}

/* ----------------------------------------------------------------------- */

/* write out a version numbered header */
static error filenamedb__write_header(os_fw f)
{
  static const char comments[] = "# Filenames";

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

error filenamedb__create(const char *filename)
{
  error                  err;
  fileswitch_object_type object_type;

  /* write out an empty db, if it doesn't already exit */

  object_type = osfile_read_no_path(filename, NULL, NULL, NULL, NULL);
  if (object_type == fileswitch_NOT_FOUND)
  {
    os_fw f;

    f = osfind_openoutw(osfind_NO_PATH, filename, NULL);
    if (f == 0)
      return error_FILENAMEDB_COULDNT_OPEN_FILE;

    err = filenamedb__write_header(f);

    osfind_close(f);

    xosfile_set_type(filename, osfile_TYPE_TEXT);

    if (err)
      goto Failure;
  }

  return error_OK;


Failure:

  return err;
}

void filenamedb__delete(const char *filename)
{
  osfile_delete(filename, NULL, NULL, NULL, NULL);
}

/* ----------------------------------------------------------------------- */

struct filenamedb_t
{
  char             *filename;

  atom_set_t       *filenames;
  hash_t           *hash;

  struct
  {
    error (*fn)(filenamedb_t *, char *);
  }
  parse;
};

static error filenamedb__parse_line(filenamedb_t *db, char *buf)
{
  error  err;
  char  *sp;

  sp = strchr(buf, ' ');
  if (!sp)
    return error_FILENAMEDB_SYNTAX_ERROR;

  *sp++ = '\0'; /* skip space */

  err = filenamedb__add(db, buf, sp);
  if (err)
    return err;

  return error_OK;
}

static error filenamedb__parse_first_line(filenamedb_t *db, char *buf)
{
  /* validate the db signature */

  if (strcmp(buf, signature) != 0)
    return error_FILENAMEDB_INCOMPATIBLE;

  db->parse.fn = filenamedb__parse_line;

  return error_OK;
}

// exactly the same as in tagdb
static error filenamedb__read_db(filenamedb_t *db)
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
    return error_FILENAMEDB_COULDNT_OPEN_FILE;

  db->parse.fn = filenamedb__parse_first_line;

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

error filenamedb__open(const char *filename, filenamedb_t **pdb)
{
  error         err;
  char         *filenamecopy = NULL;
  atom_set_t   *filenames    = NULL;
  hash_t       *hash         = NULL;
  filenamedb_t *db           = NULL;

  filenamecopy = str_dup(filename);
  if (filenamecopy == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  /* I could use the same dict for both the IDs and the filenames, but as I'm
   * writing this I'm noticing that the IDs are presently always of a
   * constant length. They could therefore live in their own dedicated array.
   * Perhaps a specialised version of dict with fixed-length entries. So I'm
   * keeping them separate for now. */

  filenames = atom_create_tuned(32768 / 80, 32768); /* est. 80 chars/entry */
  if (filenames == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  err = hash__create(HASHSIZE,
                     digestdb_hash,
                     digestdb_compare,
                     no_destroy,
                     no_destroy,
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
  db->filenames   = filenames;
  db->hash        = hash;

  /* read the database in */
  err = filenamedb__read_db(db);
  if (err)
    goto Failure;

  *pdb = db;

  return error_OK;


Failure:

  free(db);
  hash__destroy(hash);
  atom_destroy(filenames);
  free(filenamecopy);

  return err;
}

void filenamedb__close(filenamedb_t *db)
{
  if (db == NULL)
    return;

  filenamedb__commit(db);

  hash__destroy(db->hash);
  atom_destroy(db->filenames);

  free(db->filename);

  free(db);
}

/* ----------------------------------------------------------------------- */

struct commit_state
{
  filenamedb_t *db;
  char         *buf;
  size_t        bufsz;
  os_fw         f;
};

static int commit_cb(const void *key, const void *value, void *arg)
{
  struct commit_state *state = arg;
  const unsigned char *k;
  const char          *v;
  char                 ktext[33];
  int                  c;

  k = key;
  v = value;

  digestdb_encode(ktext, k);
  ktext[32] = '\0';

  c = sprintf(state->buf, "%s %s\n", ktext, v);

  /* 'c' does not include the terminator */

  osgbpb_writew(state->f, (byte *) state->buf, c);

  return 0;
}

error filenamedb__commit(filenamedb_t *db)
{
  error               err;
  struct commit_state state;

  state.db = db;
  state.f  = 0;

  state.buf = malloc(WRITEBUFSZ);
  if (state.buf == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  state.bufsz = WRITEBUFSZ;

  state.f = osfind_openoutw(osfind_NO_PATH, db->filename, NULL);
  if (state.f == 0)
    return error_FILENAMEDB_COULDNT_OPEN_FILE;

  err = filenamedb__write_header(state.f);
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

error filenamedb__add(filenamedb_t *db,
                      const char   *id,
                      const char   *filename)
{
  error                err;
  unsigned char        hash[16];
  int                  kindex;
  atom_t               vindex;
  const unsigned char *key;
  const char          *value;

  /* convert ID from ASCII hex to binary */
  digestdb_decode(hash, id);

  err = digestdb_add(hash, &kindex);
  if (err)
    return err;

  key = digestdb_get(kindex);

  err = atom_new(db->filenames, (const unsigned char *) filename,
                 strlen(filename) + 1, &vindex);
  if (err == error_ATOM_NAME_EXISTS)
    err = error_OK;
  else if (err)
    return err;

  value = (const char *) atom_get(db->filenames, vindex, NULL);

  /* this will update the value if the key is already present */

  hash__insert(db->hash, (unsigned char *) key, (char *) value);

  return error_OK;
}

/* ----------------------------------------------------------------------- */

const char *filenamedb__get(filenamedb_t *db,
                            const char   *id)
{
  return hash__lookup(db->hash, id);
}

/* ----------------------------------------------------------------------- */

static int prune_cb(const void *key, const void *value, void *arg)
{
  filenamedb_t           *db = arg;
  fileswitch_object_type  object_type;

  /* does the file exist? */
  object_type = osfile_read_no_path(value, NULL, NULL, NULL, NULL);
  if (object_type == fileswitch_NOT_FOUND)
  {
    /* if not, delete it */
    hash__remove(db->hash, key);
  }

  return 0;
}

error filenamedb__prune(filenamedb_t *db)
{
  hash__walk(db->hash, prune_cb, db);

  return error_OK;
}
