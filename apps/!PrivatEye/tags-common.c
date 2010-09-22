/* --------------------------------------------------------------------------
 *    Name: tags-common.c
 * Purpose: Common tags behaviour
 * Version: $Id: tags-common.c,v 1.8 2010-01-29 15:09:00 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifdef EYE_TAGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/hourglass.h"
#include "oslib/osfile.h"
#include "oslib/osfscontrol.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/databases/filename-db.h"
#include "appengine/databases/tag-db.h"
#include "appengine/gadgets/tag-cloud.h"
#include "appengine/graphics/image.h"
#include "appengine/io/md5.h"

#include "privateeye.h"

#include "tags-common.h"

#define TAGS_DIR                "<Choices$Write>." APPNAME ".Tags"
#define TAGDB_FILE              TAGS_DIR ".Tags"
#define FILENAMEDB_FILE         TAGS_DIR ".Filenames"

#define TAGS_BACKUP_DIR         TAGS_DIR ".Backups"
#define TAGDB_BACKUP_FILE       TAGS_BACKUP_DIR ".Tags"
#define FILENAMEDB_BACKUP_FILE  TAGS_BACKUP_DIR ".Filenames"

/* ----------------------------------------------------------------------- */

static struct
{
  tagdb        *db;        /* tags' digests */
  filenamedb_t *fdb;       /* maps digests to filenames */
  int           backed_up; /* have we backed up this session? */
}
LOCALS;

tagdb *tags_common__get_db(void)
{
  return LOCALS.db;
}

filenamedb_t *tags_common__get_filename_db(void)
{
  return LOCALS.fdb;
}

/* ----------------------------------------------------------------------- */

/* FIXME: This is probably in the wrong place. */
void tags_common__choices_updated(const choices *cs)
{
  NOT_USED(cs);

  /* for (each tag cloud)
       if (LOCALS.tc)
         set_config(LOCALS.tc); */
}

/* ----------------------------------------------------------------------- */

static void backup(void)
{
  os_error *err;

  if (LOCALS.backed_up)
    return;

  hourglass_on();

  /* backup the databases */

  err = xosfile_create_dir(TAGS_DIR, 0);
  if (err)
    goto exit;

  err = xosfile_create_dir(TAGS_BACKUP_DIR, 0);
  if (err)
    goto exit;

  err = xosfscontrol_copy(TAGDB_FILE, TAGDB_BACKUP_FILE,
                          osfscontrol_COPY_FORCE, 0, 0, 0, 0, NULL);
  if (err)
    goto exit;

  err = xosfscontrol_copy(FILENAMEDB_FILE, FILENAMEDB_BACKUP_FILE,
                          osfscontrol_COPY_FORCE, 0, 0, 0, 0, NULL);
  if (err)
    goto exit;

  hourglass_off();

  LOCALS.backed_up = 1;

exit:

  return;
}

/* ----------------------------------------------------------------------- */

error tags_common__add_tag(tag_cloud  *tc,
                           const char *name,
                           int         length,
                           void       *arg)
{
  error  err;
  tagdb *db = arg;

  NOT_USED(length);

  tagdb__add(db, name, NULL);

  err = tags_common__set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common__delete_tag(tag_cloud *tc,
                              int        index,
                              void      *arg)
{
  error  err;
  tagdb *db = arg;

  tagdb__remove(db, index);

  err = tags_common__set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common__rename_tag(tag_cloud  *tc,
                              int         index,
                              const char *name,
                              int         length,
                              void       *arg)
{
  error  err;
  tagdb *db = arg;

  NOT_USED(length);

  err = tagdb__rename(db, index, name);
  if (err)
    return err;

  err = tags_common__set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common__tag(tag_cloud  *tc,
                       int         index,
                       const char *digest,
                       const char *file_name,
                       void       *arg)
{
  error  err;
  tagdb *db = arg;

  err = tagdb__tagid(db, digest, index);
  if (err)
    return err;

  err = filenamedb__add(LOCALS.fdb, digest, file_name);
  if (err)
    return err;

  printf("tag: added %s:%s\n", digest, file_name);

  err = tags_common__set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common__detag(tag_cloud  *tc,
                         int         index,
                         const char *digest,
                         void       *arg)
{
  error  err;
  tagdb *db = arg;

  err = tagdb__untagid(db, digest, index);
  if (err)
    return err;

  /* We _don't_ remove from the filenamedb here, as there may be other tags
   * applied to the same file. */

  err = tags_common__set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common__tagfile(tag_cloud  *tc,
                           const char *file_name,
                           int         index,
                           void       *arg)
{
  error  err;
  tagdb *db = arg;
  char   digest[33];

  err = md5__from_file(file_name, digest);
  if (err)
    return err;

  err = tagdb__tagid(db, digest, index);
  if (err)
    return err;

  err = filenamedb__add(LOCALS.fdb, digest, file_name);
  if (err)
    return err;

  err = tags_common__set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common__detagfile(tag_cloud  *tc,
                             const char *file_name,
                             int         index,
                             void       *arg)
{
  error  err;
  tagdb *db = arg;
  char   digest[33];

  err = md5__from_file(file_name, digest);
  if (err)
    return err;

  err = tagdb__untagid(db, digest, index);
  if (err)
    return err;

  err = tags_common__set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common__event(tag_cloud        *tc,
                         tag_cloud__event  event,
                         void             *arg)
{
  NOT_USED(tc);
  NOT_USED(arg);

  switch (event)
  {
  case tag_cloud__EVENT_COMMIT:

    /* Backup before we write out the databases. */
    backup();

    filenamedb__commit(LOCALS.fdb);

    tagdb__commit(LOCALS.db);

    break;
  }

  return error_OK;
}

/* ----------------------------------------------------------------------- */

error tags_common__set_tags(tag_cloud *tc)
{
  error           err;
  int             tagsallocated;
  tag_cloud__tag *tags;
  int             bufallocated;
  char           *buf;
  char           *bufp;
  char           *bufend;
  int             cont;
  int             ntags;
  tagdb__tag      tag;
  int             count;
  tag_cloud__tag *t;

  tagsallocated = 0; /* allocated */
  tags          = NULL;

  bufallocated  = 0;
  buf           = NULL;
  bufp          = buf;
  bufend        = buf;

  cont  = 0;
  ntags = 0;
  for (;;)
  {
    err = tagdb__enumerate_tags(LOCALS.db, &cont, &tag, &count);
    if (err)
      goto failure;

    if (cont == 0)
      break; /* none left */

    do
    {
      err = tagdb__tagtoname(LOCALS.db, tag, bufp, bufend - bufp);
      if (err == error_TAGDB_BUFF_OVERFLOW)
      {
        int   n;
        char *newbuf;

        n = bufallocated * 2;
        if (n < 128)
          n = 128;

        newbuf = realloc(buf, n * sizeof(*newbuf));
        if (newbuf == NULL)
        {
          err = error_OOM;
          goto failure;
        }

        /* adjust bufp */
        bufp += newbuf - buf;

        buf          = newbuf;
        bufallocated = n;
        bufend       = newbuf + bufallocated;
      }
      else if (err)
      {
        goto failure;
      }
    }
    while (err == error_TAGDB_BUFF_OVERFLOW);

    if (ntags >= tagsallocated)
    {
      int             n;
      tag_cloud__tag *newtags;

      n = tagsallocated * 2;
      if (n < 8)
        n = 8;

      newtags = realloc(tags, n * sizeof(*newtags));
      if (newtags == NULL)
      {
        err = error_OOM;
        goto failure;
      }

      tags          = newtags;
      tagsallocated = n;
    }

    tags[ntags].name  = (void *) (bufp - buf); /* store as a delta now, fix
                                                  up later */
    tags[ntags].count = count;
    ntags++;

    bufp += strlen(bufp) + 1;
  }

  /* We've stored the tag name pointers as deltas so we can cope when the
   * block moves. We now fix them all up. */

  for (t = tags; t < tags + ntags; t++)
    t->name = buf + (int) t->name;

  err = tag_cloud__set_tags(tc, tags, ntags);
  if (err)
    goto failure;

  free(buf);

  free(tags);

  return error_OK;


failure:

  return err;
}

/* ----------------------------------------------------------------------- */

error tags_common__set_highlights(tag_cloud *tc, image *image)
{
  error       err;
  char        digest[33];
  int        *indices;
  int         nindices;
  int         allocated;
  int         cont;
  tagdb__tag  tag;

  /* DON'T use LOCALS.image here -- it may not be set up, yet */

  if (image == NULL)
    return error_OK;

  err = image_get_md5(image, digest);
  if (err)
    goto failure;

  indices   = NULL;
  nindices  = 0;
  allocated = 0;

  cont = 0;

  do
  {
    err = tagdb__get_tags_for_id(LOCALS.db, digest, &cont, &tag);
    if (err == error_TAGDB_UNKNOWN_ID)
      break;
    else if (err)
      goto failure;

    if (cont)
    {
      if (nindices >= allocated)
      {
        int *newindices;
        int  newallocated;

        newallocated = allocated * 2;
        if (newallocated < 8)
          newallocated = 8;

        newindices = realloc(indices, newallocated * sizeof(*indices));
        if (newindices == NULL)
        {
          err = error_OOM;
          goto failure;
        }

        indices   = newindices;
        allocated = newallocated;
      }

      indices[nindices++] = tag;
    }
  }
  while (cont);

  err = tag_cloud__highlight(tc, indices, nindices);
  if (err)
    goto failure;

  free(indices);

  return error_OK;


failure:

  return err;
}

error tags_common__clear_highlights(tag_cloud *tc)
{
  return tag_cloud__highlight(tc, NULL, 0);
}

/* ----------------------------------------------------------------------- */

static int tags_common__refcount = 0;

error tags_common__init(void)
{
  error err;

  if (tags_common__refcount++ == 0)
  {
    /* dependencies */

    err = tagdb__init();
    if (err)
      goto failure;

    err = filenamedb__init();
    if (err)
    {
      tagdb__fin();
      goto failure;
    }

    err = tag_cloud__init();
    if (err)
    {
      filenamedb__fin();
      tagdb__fin();
      goto failure;
    }
  }

  return error_OK;


failure:

  return err;
}

void tags_common__fin(void)
{
  if (--tags_common__refcount == 0)
  {
    tags_common__properfin(1); /* force shutdown */

    tag_cloud__fin();

    filenamedb__fin();

    tagdb__fin();
  }
}

/* ----------------------------------------------------------------------- */

/* The 'proper' init/fin functions provide lazy initialisation. */

static int tags_common__properrefcount = 0;

error tags_common__properinit(void)
{
  error err;

  if (tags_common__properrefcount++ == 0)
  {
    os_error     *oserr;
    tagdb        *db  = NULL;
    filenamedb_t *fdb = NULL;

    hourglass_on();

    /* init */

    oserr = xosfile_create_dir(TAGS_DIR, 0);
    if (oserr)
    {
      err = error_OS;
      goto failure;
    }

    err = tagdb__create(TAGDB_FILE);
    if (err)
      goto failure;

    err = tagdb__open(TAGDB_FILE, &db);
    if (err)
      goto failure;

    err = filenamedb__create(FILENAMEDB_FILE);
    if (err)
      goto failure;

    err = filenamedb__open(FILENAMEDB_FILE, &fdb);
    if (err)
      goto failure;

    LOCALS.db  = db;
    LOCALS.fdb = fdb;

    hourglass_off();
  }

  return error_OK;


failure:

  hourglass_off();

  /* FIXME: Cleanup code is missing. */

  return err;
}

void tags_common__properfin(int force)
{
  /* allow a forced shutdown only if we're not at refcount zero */

  if (tags_common__properrefcount == 0)
    return;

  if (force)
    tags_common__properrefcount = 1;

  if (--tags_common__properrefcount == 0)
  {
    /* backup before we write out the databases */
    backup();

    filenamedb__close(LOCALS.fdb);

    tagdb__close(LOCALS.db);
  }
}

/* ----------------------------------------------------------------------- */

#else

extern int dummy;

#endif /* EYE_TAGS */
