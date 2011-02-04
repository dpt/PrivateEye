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
#include "appengine/datastruct/hash.h"
#include "appengine/base/strings.h"

#include "appengine/databases/filename-db.h"

static const char signature[] = "1";

/* ----------------------------------------------------------------------- */

error filenamedb__init(void)
{
  return error_OK;
}

void filenamedb__fin(void)
{
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

  hash_t           *ids;

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

  bufsz = 1024;
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

error filenamedb__open(const char *filename, filenamedb_t **pdb)
{
  error         err;
  char         *filenamecopy = NULL;
  hash_t       *ids          = NULL;
  filenamedb_t *db           = NULL;

  filenamecopy = str_dup(filename);
  if (filenamecopy == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  err = hash__create(97,
                     NULL, /* use default hash function */
                     NULL, /* use default string compare */
                     NULL, /* use default destroy_key function */
                     NULL, /* use default destroy_value function */
                    &ids);
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

  /* read the database in */
  err = filenamedb__read_db(db);
  if (err)
    goto Failure;

  *pdb = db;

  return error_OK;


Failure:

  free(db);
  hash__destroy(ids);
  free(filenamecopy);

  return err;
}

void filenamedb__close(filenamedb_t *db)
{
  if (db == NULL)
    return;

  filenamedb__commit(db);

  hash__destroy(db->ids);

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
  const char          *k;
  const char          *v;
  int                  c;

  k = key;
  v = value;

  c = sprintf(state->buf, "%s %s\n", k, v);

  /* 'c' does not include the terminator */

  osgbpb_writew(state->f, (byte *) state->buf, c);

  return 0;
}

error filenamedb__commit(filenamedb_t *db)
{
  error               err;
  struct commit_state state;

  state.db = db;

  state.buf = malloc(1024); // Careful Now
  if (state.buf == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  state.bufsz = 1024;

  state.f = osfind_openoutw(osfind_NO_PATH, db->filename, NULL);
  if (state.f == 0)
    return error_FILENAMEDB_COULDNT_OPEN_FILE;

  err =  filenamedb__write_header(state.f);
  if (err)
    goto Failure;

  hash__walk(db->ids, commit_cb, &state);

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
  char *key;
  char *val;

  key = str_dup(id);
  val = str_dup(filename);
  if (key == NULL || val == NULL)
  {
    free(key);
    free(val);
    return error_OOM;
  }

  /* this will update the value if the key is already present */

  hash__insert(db->ids, key, val);

#ifndef NDEBUG
  printf("filenamedb__add: added %s:%s\n", id, filename);
#endif

  return error_OK;
}

/* ----------------------------------------------------------------------- */

const char *filenamedb__get(filenamedb_t *db,
                            const char   *id)
{
  return hash__lookup(db->ids, id);
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
    hash__remove(db->ids, key);
  }

  return 0;
}

error filenamedb__prune(filenamedb_t *db)
{
  hash__walk(db->ids, prune_cb, db);

  return error_OK;
}
