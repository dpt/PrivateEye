/* --------------------------------------------------------------------------
 *    Name: tags-common.c
 * Purpose: Common tags behaviour
 * ----------------------------------------------------------------------- */

#ifdef EYE_TAGS

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/hourglass.h"
#include "oslib/osfile.h"
#include "oslib/osfscontrol.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/databases/digest-db.h"
#include "appengine/databases/filename-db.h"
#include "appengine/databases/tag-db.h"
#include "appengine/gadgets/tag-cloud.h"
#include "appengine/graphics/image.h"
#include "appengine/io/md5.h"

#include "privateeye.h"

#include "tags-common.h"

#define TAGS_DIR               "<Choices$Write>." APPNAME ".Tags"
#define TAGS_SECTION           TAGS_DIR ".Default"
#define TAGDB_FILE             TAGS_SECTION ".Tags"
#define FILENAMEDB_FILE        TAGS_SECTION ".Filenames"

#define TAGS_BACKUP_DIR        TAGS_SECTION ".Backups"
#define TAGDB_BACKUP_FILE      TAGS_BACKUP_DIR ".Tags"
#define FILENAMEDB_BACKUP_FILE TAGS_BACKUP_DIR ".Filenames"

/* ----------------------------------------------------------------------- */

static struct
{
  tagdb        *db;        /* maps digests to tags */
  filenamedb_t *fdb;       /* maps digests to filenames */
  int           backed_up; /* have we backed up this session? */
}
LOCALS;

tagdb *tags_common_get_db(void)
{
  return LOCALS.db;
}

filenamedb_t *tags_common_get_filename_db(void)
{
  return LOCALS.fdb;
}

/* ----------------------------------------------------------------------- */

/* FIXME: This is probably in the wrong place. */
void tags_common_choices_updated(const choices *cs)
{
  NOT_USED(cs);

  /* for (each tag cloud)
       if (LOCALS.tc)
         set_config(LOCALS.tc); */
}

/* ----------------------------------------------------------------------- */

static os_error *forcecopy(const char *src, const char *dst)
{
  return xosfscontrol_copy(src,
                           dst,
                           osfscontrol_COPY_FORCE,
                           0,
                           0,
                           0,
                           0,
                           NULL);
}

static void backup(void)
{
  os_error *err;

  if (LOCALS.backed_up)
    return;

  hourglass_on();

  /* backup the databases */

  err = xosfile_create_dir(TAGS_BACKUP_DIR, 0);
  if (err)
    goto exit;

  err = forcecopy(TAGDB_FILE, TAGDB_BACKUP_FILE);
  if (err)
    goto exit;

  err = forcecopy(FILENAMEDB_FILE, FILENAMEDB_BACKUP_FILE);
  if (err)
    goto exit;

  hourglass_off();

  LOCALS.backed_up = 1;

exit:

  return;
}

/* ----------------------------------------------------------------------- */

error tags_common_add_tag(tag_cloud  *tc,
                          const char *name,
                          int         length,
                          void       *opaque)
{
  error  err;
  tagdb *db = opaque;

  NOT_USED(length);

  tagdb_add(db, name, NULL);

  err = tags_common_set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common_delete_tag(tag_cloud *tc, int index, void *opaque)
{
  error  err;
  tagdb *db = opaque;

  tagdb_remove(db, index);

  err = tags_common_set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common_rename_tag(tag_cloud  *tc,
                             int         index,
                             const char *name,
                             int         length,
                             void       *opaque)
{
  error  err;
  tagdb *db = opaque;

  NOT_USED(length);

  err = tagdb_rename(db, index, name);
  if (err)
    return err;

  err = tags_common_set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common_tag(tag_cloud  *tc,
                      int         index,
                      const char *digest,
                      const char *file_name,
                      void       *opaque)
{
  error  err;
  tagdb *db = opaque;

  err = tagdb_tagid(db, digest, index);
  if (err)
    return err;

  err = filenamedb_add(LOCALS.fdb, digest, file_name);
  if (err)
    return err;

  err = tags_common_set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common_detag(tag_cloud  *tc,
                        int         index,
                        const char *digest,
                        void       *opaque)
{
  error  err;
  tagdb *db = opaque;

  err = tagdb_untagid(db, digest, index);
  if (err)
    return err;

  /* We _don't_ remove from the filenamedb here, as there may be other tags
   * applied to the same file. */

  err = tags_common_set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common_tagfile(tag_cloud  *tc,
                          const char *file_name,
                          int         index,
                          void       *opaque)
{
  error          err;
  tagdb         *db = opaque;
  unsigned char  digest[md5_DIGESTSZ];

  assert(md5_DIGESTSZ == digestdb_DIGESTSZ);

  err = md5_from_file(file_name, digest);
  if (err)
    return err;

  err = tagdb_tagid(db, (char *) digest, index);
  if (err)
    return err;

  err = filenamedb_add(LOCALS.fdb, (char *) digest, file_name);
  if (err)
    return err;

  err = tags_common_set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common_detagfile(tag_cloud  *tc,
                            const char *file_name,
                            int         index,
                            void       *opaque)
{
  error          err;
  tagdb         *db = opaque;
  unsigned char  digest[md5_DIGESTSZ];

  assert(md5_DIGESTSZ == digestdb_DIGESTSZ);

  err = md5_from_file(file_name, digest);
  if (err)
    return err;

  err = tagdb_untagid(db, (char *) digest, index);
  if (err)
    return err;

  err = tags_common_set_tags(tc);
  if (err)
    return err;

  return error_OK;
}

error tags_common_event(tag_cloud *tc, tag_cloud_event event, void *opaque)
{
  NOT_USED(tc);
  NOT_USED(opaque);

  switch (event)
  {
  case tag_cloud_EVENT_COMMIT:

    /* Backup before we write out the databases. */
    backup();

    filenamedb_commit(LOCALS.fdb);

    tagdb_commit(LOCALS.db);

    break;
  }

  return error_OK;
}

/* ----------------------------------------------------------------------- */

error tags_common_set_tags(tag_cloud *tc)
{
  error          err;
  int            tagsallocated;
  tag_cloud_tag *tags;
  int            bufallocated;
  char          *buf;
  char          *bufp;
  char          *bufend;
  int            cont;
  int            ntags;
  tagdb_tag      tag;
  int            count;
  tag_cloud_tag *t;

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
    err = tagdb_enumerate_tags(LOCALS.db, &cont, &tag, &count);
    if (err)
      goto failure;

    if (cont == 0)
      break; /* none left */

    do
    {
      err = tagdb_tagtoname(LOCALS.db, tag, bufp, bufend - bufp);
      if (err == error_TAGDB_BUFF_OVERFLOW)
      {
        int   n;
        char *newbuf;

        n = bufallocated * 2;
        if (n < 128) // FIXME: Hoist growth constants.
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
      tag_cloud_tag *newtags;

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

    /* store as a delta now, fix up later */
    tags[ntags].name  = (void *) (bufp - buf);
    tags[ntags].count = count;
    ntags++;

    bufp += strlen(bufp) + 1;
  }

  /* We've stored the tag name pointers as deltas so we can cope when the
   * block moves. We now fix them all up. */

  for (t = tags; t < tags + ntags; t++)
    t->name = buf + (int) t->name;

  err = tag_cloud_set_tags(tc, tags, ntags);
  if (err)
    goto failure;

  free(buf);

  free(tags);

  return error_OK;


failure:

  return err;
}

/* ----------------------------------------------------------------------- */

error tags_common_set_highlights(tag_cloud *tc, image_t *image)
{
  error         err;
  unsigned char digest[image_DIGESTSZ];
  int          *indices;
  int           nindices;
  int           allocated;
  int           cont;
  tagdb_tag     tag;

  /* DON'T use LOCALS.image here -- it may not be set up, yet */

  if (image == NULL)
    return error_OK;

  err = image_get_digest(image, digest);
  if (err)
    goto failure;

  indices   = NULL;
  nindices  = 0;
  allocated = 0;

  cont = 0;

  do
  {
    err = tagdb_get_tags_for_id(LOCALS.db, (char *) digest, &cont, &tag);
    if (err == error_TAGDB_UNKNOWN_ID)
      break;
    else if (err)
      goto failure;

    if (cont) // FIXME: if (!cont) break; etc.
    {
      if (nindices >= allocated)
      {
        int *newindices;
        int  newallocated;

        newallocated = allocated * 2;
        if (newallocated < 8) // FIXME: Hoist growth constants.
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

  err = tag_cloud_highlight(tc, indices, nindices);
  if (err)
    goto failure;

  free(indices);

  return error_OK;


failure:

  return err;
}

error tags_common_clear_highlights(tag_cloud *tc)
{
  return tag_cloud_highlight(tc, NULL, 0);
}

/* ----------------------------------------------------------------------- */

static int tags_common_refcount = 0;

error tags_common_init(void)
{
  error err;

  if (tags_common_refcount++ == 0)
  {
    /* dependencies */

    err = tagdb_init();
    if (err)
      goto failure;

    err = filenamedb_init();
    if (err)
    {
      tagdb_fin();
      goto failure;
    }

    err = tag_cloud_init();
    if (err)
    {
      filenamedb_fin();
      tagdb_fin();
      goto failure;
    }
  }

  return error_OK;


failure:

  return err;
}

void tags_common_fin(void)
{
  if (--tags_common_refcount == 0)
  {
    tags_common_lazyfin(1); /* force shutdown */

    tag_cloud_fin();

    filenamedb_fin();

    tagdb_fin();
  }
}

/* ----------------------------------------------------------------------- */

/* The 'lazy' init/fin functions provide lazy initialisation. */

static int tags_common_lazyrefcount = 0;

error tags_common_lazyinit(void)
{
  error err;

  if (tags_common_lazyrefcount++ == 0)
  {
    os_error     *oserr;
    tagdb        *db  = NULL;
    filenamedb_t *fdb = NULL;

    hourglass_on();

    /* init */

    oserr = xosfile_create_dir(TAGS_DIR, 0);
    if (oserr == NULL)
      oserr = xosfile_create_dir(TAGS_SECTION, 0);
    if (oserr)
    {
      err = error_OS;
      goto failure;
    }

    err = tagdb_create(TAGDB_FILE);
    if (err)
      goto failure;

    err = tagdb_open(TAGDB_FILE, &db);
    if (err)
      goto failure;

    err = filenamedb_create(FILENAMEDB_FILE);
    if (err)
      goto failure;

    err = filenamedb_open(FILENAMEDB_FILE, &fdb);
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

void tags_common_lazyfin(int force)
{
  /* allow a forced shutdown only if we're not at refcount zero */

  if (tags_common_lazyrefcount == 0)
    return;

  if (force)
    tags_common_lazyrefcount = 1;

  if (--tags_common_lazyrefcount == 0)
  {
    /* backup before we write out the databases */
    backup();

    filenamedb_close(LOCALS.fdb);

    tagdb_close(LOCALS.db);
  }
}

/* ----------------------------------------------------------------------- */

#else

extern int dummy;

#endif /* EYE_TAGS */
