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

#include "databases/digest-db.h"
#include "databases/filename-db.h"
#include "databases/tag-db.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/datastruct/array.h"
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
  tagdb_t      *db;        /* maps digests to tags */
  filenamedb_t *fdb;       /* maps digests to filenames */
  int           backed_up; /* have we backed up during this session? */
}
LOCALS;

/* ----------------------------------------------------------------------- */

tagdb_t *tags_common_get_db(void)
{
  return LOCALS.db;
}

filenamedb_t *tags_common_get_filename_db(void)
{
  return LOCALS.fdb;
}

/* ----------------------------------------------------------------------- */

// unused
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

result_t tags_common_add_tag(tag_cloud  *tc,
                             const char *name,
                             int         length,
                             void       *opaque)
{
  result_t     err;
  tags_common *common = opaque;

  NOT_USED(length);

  tagdb_add(common->db, name, NULL);

  err = tags_common_set_tags(tc, common);
  if (err)
    return err;

  return result_OK;
}

result_t tags_common_delete_tag(tag_cloud *tc, int index, void *opaque)
{
  result_t     err;
  tags_common *common = opaque;

  tagdb_remove(common->db, common->indextotag[index]);

  err = tags_common_set_tags(tc, common);
  if (err)
    return err;

  return result_OK;
}

result_t tags_common_rename_tag(tag_cloud  *tc,
                                int         index,
                                const char *name,
                                int         length,
                                void       *opaque)
{
  result_t     err;
  tags_common *common = opaque;

  NOT_USED(length);

  err = tagdb_rename(common->db, common->indextotag[index], name);
  if (err)
    return err;

  err = tags_common_set_tags(tc, common);
  if (err)
    return err;

  return result_OK;
}

result_t tags_common_tag(tag_cloud  *tc,
                         int         index,
                         const char *digest,
                         const char *file_name,
                         void       *opaque)
{
  result_t     err;
  tags_common *common = opaque;

  err = tagdb_tagid(common->db, digest, common->indextotag[index]);
  if (err)
    return err;

  err = filenamedb_add(LOCALS.fdb, digest, file_name);
  if (err)
    return err;

  err = tags_common_set_tags(tc, common);
  if (err)
    return err;

  return result_OK;
}

result_t tags_common_detag(tag_cloud  *tc,
                           int         index,
                           const char *digest,
                           void       *opaque)
{
  result_t     err;
  tags_common *common = opaque;

  err = tagdb_untagid(common->db, digest, common->indextotag[index]);
  if (err)
    return err;

  /* We _don't_ remove from the filenamedb here, as there may be other tags
   * applied to the same file. */

  err = tags_common_set_tags(tc, common);
  if (err)
    return err;

  return result_OK;
}

result_t tags_common_tagfile(tag_cloud  *tc,
                             const char *file_name,
                             int         index,
                             void       *opaque)
{
  result_t       err;
  tags_common   *common = opaque;
  unsigned char  digest[md5_DIGESTSZ];

  assert(md5_DIGESTSZ == digestdb_DIGESTSZ);

  err = md5_from_file(file_name, digest);
  if (err)
    return err;

  err = tagdb_tagid(common->db, (char *) digest, common->indextotag[index]);
  if (err)
    return err;

  err = filenamedb_add(LOCALS.fdb, (char *) digest, file_name);
  if (err)
    return err;

  err = tags_common_set_tags(tc, common);
  if (err)
    return err;

  return result_OK;
}

result_t tags_common_detagfile(tag_cloud  *tc,
                               const char *file_name,
                               int         index,
                               void       *opaque)
{
  result_t       err;
  tags_common   *common = opaque;
  unsigned char  digest[md5_DIGESTSZ];

  assert(md5_DIGESTSZ == digestdb_DIGESTSZ);

  err = md5_from_file(file_name, digest);
  if (err)
    return err;

  err = tagdb_untagid(common->db, (char *) digest, common->indextotag[index]);
  if (err)
    return err;

  err = tags_common_set_tags(tc, common);
  if (err)
    return err;

  return result_OK;
}

result_t tags_common_event(tag_cloud *tc, tag_cloud_event event, void *opaque)
{
  NOT_USED(tc);
  NOT_USED(opaque);

  switch (event)
  {
  case tag_cloud_EVENT_COMMIT:

    /* Backup before we write out the databases. */
    backup(); // would it be best to backup at start of session only?

    filenamedb_commit(LOCALS.fdb);

    tagdb_commit(LOCALS.db);

    break;
  }

  return result_OK;
}

/* ----------------------------------------------------------------------- */

#define BUFMIN 128 /* minimum size of name buffer */
#define TAGMIN 8   /* minimum size of tags array */

result_t tags_common_set_tags(tag_cloud *tc, tags_common *common)
{
  result_t       err;
  int            tagsallocated;
  tag_cloud_tag *tags;
  int            bufallocated;
  char          *buf;
  char          *bufp;
  char          *bufend;
  int            cont;
  int            ntags;
  tag_cloud_tag *t;
  int            ittallocated;
  tagdb_tag_t   *indextotag;

  tagsallocated = 0; /* allocated */
  tags          = NULL;

  bufallocated  = 0;
  buf           = NULL;
  bufp          = buf;
  bufend        = buf;

  ittallocated  = 0;
  indextotag    = NULL;

  // free indextotag map
  free(common->indextotag);
  common->indextotag = NULL;

  cont  = 0;
  ntags = 0;
  for (;;)
  {
    tagdb_tag_t tag;
    int         count;
    size_t      length;

    err = tagdb_enumerate_tags(common->db, &cont, &tag, &count);
    if (err)
      goto failure;

    if (cont == 0)
      break; /* none left */

    do
    {
      err = tagdb_tagtoname(common->db, tag, bufp, &length, bufend - bufp);
      if (err == result_TAGDB_BUFF_OVERFLOW)
      {
        char *oldbuf;

        oldbuf = buf;

        if (array_grow((void **) &buf,
                       sizeof(*buf),
                       bufp - buf, /* used */
                       &bufallocated,
                       length,
                       BUFMIN))
        {
          err = result_OOM;
          goto failure;
        }

        /* adjust buffer interior pointers */
        bufp   += buf - oldbuf;
        bufend  = buf + bufallocated;
      }
      else if (err)
      {
        goto failure;
      }
    }
    while (err == result_TAGDB_BUFF_OVERFLOW);

    if (array_grow((void **) &tags,
                   sizeof(*tags),
                   ntags, /* used */
                   &tagsallocated,
                   1, /* need */
                   TAGMIN))
    {
      err = result_OOM;
      goto failure;
    }

    if (array_grow((void **) &indextotag,
                   sizeof(*indextotag),
                   ntags, /* used */
                   &ittallocated,
                   1, /* need */
                   TAGMIN))
    {
      err = result_OOM;
      goto failure;
    }

    assert(tagsallocated == ittallocated);

    length--; /* tagdb_tagtoname returns length inclusive of terminator. compensate. */

    /* store as a delta now, fix up later */
    tags[ntags].name   = (void *) (bufp - buf);
    tags[ntags].length = length;
    tags[ntags].count  = count;

    indextotag[ntags] = tag;

    ntags++;

    bufp += length;
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

  // store indextotag
  common->indextotag  = indextotag;
  common->nindextotag = ntags;

  return result_OK;


failure:

  return err;
}

/* ----------------------------------------------------------------------- */

#define HLMIN 8  /* minimum allocated size of tags array */

result_t tags_common_set_highlights(tag_cloud   *tc,
                                    image_t     *image,
                                    tags_common *common)
{
  result_t      err;
  unsigned char digest[image_DIGESTSZ];
  int          *indices;
  int           nindices;
  int           allocated;
  int           cont;
  tagdb_tag_t   tag;

  if (image == NULL)
    return result_OK;

  err = image_get_digest(image, digest);
  if (err)
    goto failure;

  indices   = NULL;
  nindices  = 0;
  allocated = 0;

  cont = 0;

  for (;;)
  {
    int i;

    err = tagdb_get_tags_for_id(common->db, (char *) digest, &cont, &tag);
    if (err == result_TAGDB_UNKNOWN_ID)
      break;
    else if (err)
      goto failure;

    if (cont == 0)
      break; /* none left */

    if (array_grow((void **) &indices,
                   sizeof(*indices),
                   nindices, /* used */
                   &allocated,
                   1,
                   HLMIN))
    {
      err = result_OOM;
      goto failure;
    }

    /* search for index */
    for (i = 0; i < common->nindextotag; i++)
      if (common->indextotag[i] == tag)
        break;

    indices[nindices++] = i;
  }

  err = tag_cloud_highlight(tc, indices, nindices);
  if (err)
    goto failure;

  free(indices);

  return result_OK;


failure:

  return err;
}

result_t tags_common_clear_highlights(tag_cloud *tc)
{
  return tag_cloud_highlight(tc, NULL, 0);
}

/* ----------------------------------------------------------------------- */

static int tags_common_refcount = 0;

result_t tags_common_init(void)
{
  result_t err;

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

  return result_OK;


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

static unsigned int tags_common_lazyrefcount = 0;

result_t tags_common_lazyinit(void)
{
  result_t err;

  if (tags_common_lazyrefcount == 0)
  {
    os_error     *oserr;
    tagdb_t      *db  = NULL;
    filenamedb_t *fdb = NULL;

    /* initialise */

    hourglass_on();

    oserr = xosfile_create_dir(TAGS_DIR, 0);
    if (oserr == NULL)
      oserr = xosfile_create_dir(TAGS_SECTION, 0);
    if (oserr)
    {
      err = result_OS;
      goto failure;
    }

    err = tagdb_open(TAGDB_FILE, &db);
    if (err)
      goto failure;

    err = filenamedb_open(FILENAMEDB_FILE, &fdb);
    if (err)
      goto failure;

    LOCALS.db      = db;
    LOCALS.fdb     = fdb;

    hourglass_off();
  }

  tags_common_lazyrefcount++;

  return result_OK;


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
