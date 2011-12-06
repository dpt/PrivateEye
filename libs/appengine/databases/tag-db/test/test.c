
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/fileswitch.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/base/strings.h"

#include "appengine/databases/tag-db.h"

/* ----------------------------------------------------------------------- */

#define FILENAME "test-tag-db"

/* ----------------------------------------------------------------------- */

static const char *tagnames[] =
{
  "marty", "jennifer", "emmett", "einstein", "lorraine", "george", "biff",
  "fred" /* do not use (to test unused tag case) */
};

static const char *ids[] =
{
  "martyjen",
  "doc",
  "einy",
  "folks",
  "all",
};

static const char *renames[] =
{
  "marty.mcfly",
  "jennifer.parker",
  "dr.emmett.brown",
  "einstein", /* no change */
  "lorraine.baines",
  "george.mcfly",
  "biff.tannen",
};

static const struct
{
  int id;
  int tag;
}
taggings[] =
{
  { 0, 0 },
  { 0, 1 },
  { 1, 2 },
  { 2, 3 },
  { 3, 4 },
  { 3, 5 },
  { 4, 0 },
  { 4, 1 },
  { 4, 2 },
  { 4, 3 },
  { 4, 4 },
  { 4, 5 },
  { 4, 6 },
};

/* ----------------------------------------------------------------------- */

typedef struct
{
  tagdb__tag tags[8]; /* 8 == NELEMS(tagnames) */
  tagdb     *db;
}
State;

/* ----------------------------------------------------------------------- */

static error test_create(State *state)
{
  error err;

  NOT_USED(state);

  err = tagdb__create(FILENAME);

  return err;
}

static error test_open(State *state)
{
  error err;

  err = tagdb__open(FILENAME, &state->db);

  return err;
}

static error test_add_tags(State *state)
{
  error err;
  int   i;

  for (i = 0; i < NELEMS(tagnames); i++)
  {
    printf("adding '%s'...", tagnames[i]);
    err = tagdb__add(state->db, tagnames[i], &state->tags[i]);
    if (err)
      goto Failure;

    printf("is tag %d\n", state->tags[i]);
  }

  return error_OK;


Failure:

  return err;
}

static error test_rename_tags(State *state)
{
  error err;
  int   i;

  for (i = 0; i < NELEMS(renames); i++)
  {
    char buf[256];

    err = tagdb__tagtoname(state->db, state->tags[i], buf, sizeof(buf));
    if (err)
      goto Failure;

    printf("renaming '%s'...", buf);

    err = tagdb__rename(state->db, state->tags[i], renames[i]);
    if (err)
      goto Failure;

    err = tagdb__tagtoname(state->db, state->tags[i], buf, sizeof(buf));
    if (err)
      goto Failure;

    printf("to '%s'\n", buf);
  }

  return error_OK;


Failure:

  return err;
}

static error test_enumerate_tags(State *state)
{
  error err;
  int   cont;
  char  buf[256];

  cont = 0;
  do
  {
    tagdb__tag tag;
    int        count;

    printf("continuation %d...", cont);

    err = tagdb__enumerate_tags(state->db, &cont, &tag, &count);
    if (err)
      goto Failure;

    if (cont)
    {
      printf("tag %d, count %d", tag, count);

      err = tagdb__tagtoname(state->db, tag, buf, sizeof(buf));
      if (err)
        goto Failure;

      printf(" ('%s')", buf);
    }

    printf("\n");
  }
  while (cont);

  return error_OK;


Failure:

  return err;
}

static error test_tag_id(State *state)
{
  error err;
  int   i;

  for (i = 0; i < NELEMS(taggings); i++)
  {
    int whichf;
    int whicht;

    whichf = taggings[i].id;
    whicht = taggings[i].tag;

    printf("tagging '%s' with %d\n", ids[whichf], state->tags[whicht]);

    err = tagdb__tagid(state->db, ids[whichf], state->tags[whicht]);
    if (err)
      goto Failure;
  }

  return error_OK;


Failure:

  return err;
}

static error test_get_tags_for_id(State *state)
{
  error err;
  int   i;
  int   cont;
  char  buf[256];

  for (i = 0; i < NELEMS(ids); i++)
  {
    int        ntags;
    tagdb__tag tag;

    printf("getting tags for '%s'... ", ids[i]);

    ntags = 0;
    cont  = 0;
    do
    {
      err = tagdb__get_tags_for_id(state->db, ids[i], &cont, &tag);
      if (err == error_TAGDB_UNKNOWN_ID)
      {
        printf("id not found ");
        continue;
      }
      else if (err)
      {
        goto Failure;
      }

      if (cont)
      {
        err = tagdb__tagtoname(state->db, tag, buf, sizeof(buf));
        if (err)
          goto Failure;

        printf("%d ('%s') ", tag, buf);
        ntags++;
      }
    }
    while (cont);

    printf("(%d tags) \n", ntags);
  }

  return error_OK;


Failure:

  return err;
}

static error test_enumerate_ids(State *state)
{
  error err;
  int   cont;
  char  buf[256];

  cont = 0;
  do
  {
    err = tagdb__enumerate_ids(state->db, &cont, buf, sizeof(buf));
    if (err)
      goto Failure;

    if (cont)
    {
      printf("- %s\n", buf);
    }
  }
  while (cont);

  return error_OK;


Failure:

  return err;
}

static error test_enumerate_ids_by_tag(State *state)
{
  error err;
  int   i;

  for (i = 0; i < NELEMS(tagnames); i++)
  {
    int  cont;
    char buf[256];

    err = tagdb__tagtoname(state->db, state->tags[i], buf, sizeof(buf));
    if (err)
      goto Failure;

    printf("ids tagged with tag id %d ('%s')...\n", i, buf);

    cont = 0;
    do
    {
      err = tagdb__enumerate_ids_by_tag(state->db, i, &cont,
                                        buf, sizeof(buf));
      if (err)
        goto Failure;

      if (cont)
      {
        printf("- %s\n", buf);
      }
    }
    while (cont);
  }

  return error_OK;


Failure:

  return err;
}

static error test_enumerate_ids_by_tags(State *state)
{
  error err;
  int   cont;
  char  buf[256];

  {
    static const tagdb__tag want[] = { 0, 1 };

    printf("ids tagged with '%s' and '%s'...\n",
           tagnames[want[0]], tagnames[want[1]]);

    cont = 0;
    do
    {
      err = tagdb__enumerate_ids_by_tags(state->db, want, NELEMS(want),
                                              &cont, buf, sizeof(buf));
      if (err)
        goto Failure;

      if (cont)
      {
        printf("- %s\n", buf);
      }
    }
    while (cont);
  }

  return error_OK;


Failure:

  return err;
}

static error test_tag_remove(State *state)
{
  int i;

  for (i = 0; i < NELEMS(state->tags); i++)
    tagdb__remove(state->db, state->tags[i]);

  return error_OK;
}

static error test_commit(State *state)
{
  error err;

  err = tagdb__commit(state->db);
  if (err)
    return err;

  return error_OK;
}

static error test_forget(State *state)
{
  int i;

  for (i = 0; i < NELEMS(ids); i++)
    tagdb__forget(state->db, ids[i]);

  return error_OK;
}

static error test_close(State *state)
{
  tagdb__close(state->db); /* remember that this calls __commit */

  return error_OK;
}

static error test_delete(State *state)
{
  NOT_USED(state);

  tagdb__delete(FILENAME);

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static int rnd(int mod)
{
  return (rand() % mod) + 1;
}

static const char *randomtagname(void)
{
  static char buf[5 + 1];

  int length;
  int i;

  length = rnd(NELEMS(buf) - 1);

  for (i = 0; i < length; i++)
    buf[i] = 'a' + rnd(26) - 1;

  buf[i] = '\0';

  return buf;
}

static const char *randomid(void)
{
  static char buf[10 + 1];

  int length;
  int i;

  length = rnd(NELEMS(buf) - 1);

  for (i = 0; i < length; i++)
    buf[i] = 'A' + rnd(26) - 1;

  buf[i] = '\0';

  return buf;
}

static error bash_enumerate(State *state)
{
  error err;
  int   cont;
  char  buf[256];

  cont = 0;
  do
  {
    err = tagdb__enumerate_ids(state->db, &cont, buf, sizeof(buf));
    if (err)
      goto failure;

    if (cont)
    {
      int        ntags;
      tagdb__tag tag;
      int        cont2;
      char       buf2[256];

      printf("getting tags for '%s'... ", buf);

      ntags = 0;
      cont2 = 0;
      do
      {
        err = tagdb__get_tags_for_id(state->db, buf, &cont2, &tag);
        if (err == error_TAGDB_UNKNOWN_ID)
        {
          printf("id not found ");
          continue;
        }
        else if (err)
        {
          goto failure;
        }

        if (cont2)
        {
          err = tagdb__tagtoname(state->db, tag, buf2, sizeof(buf2));
          if (err)
            goto failure;

          printf("%d ('%s') ", tag, buf2);
          ntags++;
        }
      }
      while (cont2);

      printf("(%d tags) \n", ntags);
    }
  }
  while (cont);

  return error_OK;


failure:

  return err;
}

static error test_bash(State *state)
{
  const int ntags       = 97; /* for now */
  const int nids        = 10; /* for now */
  const int ntaggings   = ntags * nids / 2; /* number of times to tag */
  const int nuntaggings = ntags * nids / 4; /* number of times to untag */
  const int reps        = 10; /* overall number of repetitions */

  error       err;
  int         i;
  tagdb__tag  tags[ntags];
  char       *tagnames[ntags];
  char       *idnames[nids];
  int         j;

  srand(0x6487ED51);

  printf("bash: create\n");

  err = tagdb__create(FILENAME);
  if (err)
    goto failure;

  printf("bash: open\n");

  err = tagdb__open(FILENAME, &state->db);
  if (err)
    goto failure;

  printf("bash: setup\n");

  for (i = 0; i < ntags; i++)
    tagnames[i] = NULL;

  for (i = 0; i < nids; i++)
    idnames[i] = NULL;

  for (j = 0; j < reps; j++)
  {
    printf("bash: starting loop %d\n", j);

    printf("bash: create and add tags\n");

    for (i = 0; i < ntags; i++)
    {
      const char *name;
      int         k;

      if (tagnames[i])
        continue;

      do
      {
        name = randomtagname();

        /* ensure that the random name is unique */
        for (k = 0; k < ntags; k++)
          if (tagnames[k] && strcmp(tagnames[k], name) == 0)
            break;
      }
      while (k < ntags);

      tagnames[i] = str_dup(name);
      if (tagnames[i] == NULL)
      {
        err = error_OOM;
        goto failure;
      }

      printf("adding '%s'...", tagnames[i]);

      err = tagdb__add(state->db, tagnames[i], &tags[i]);
      if (err)
        goto failure;

      printf("is tag %d\n", tags[i]);
    }

    printf("bash: create ids\n");

    for (i = 0; i < nids; i++)
    {
      const char *id;
      int         k;

      if (idnames[i])
        continue;

      do
      {
        id = randomid();

        /* ensure that the random name is unique */
        for (k = 0; k < nids; k++)
          if (idnames[k] && strcmp(idnames[k], id) == 0)
            break;
      }
      while (k < nids);

      idnames[i] = str_dup(id);
      if (idnames[i] == NULL)
      {
        err = error_OOM;
        goto failure;
      }

      printf("%d is id '%s'\n", i, idnames[i]);
    }

    printf("bash: tag random ids with random tags randomly\n");

    for (i = 0; i < ntaggings; i++)
    {
      int whichid;
      int whichtag;

      do
        whichid = rnd(nids) - 1;
      while (idnames[whichid] == NULL);

      do
        whichtag = rnd(ntags) - 1;
      while (tagnames[whichtag] == NULL);

      printf("tagging '%s' with %d\n", idnames[whichid], tags[whichtag]);

      err = tagdb__tagid(state->db, idnames[whichid], tags[whichtag]);
      if (err)
        goto failure;
    }

    printf("bash: enumerate\n");

    err = bash_enumerate(state);
    if (err)
      goto failure;

    printf("bash: rename all tags\n");

    for (i = 0; i < ntags; i++)
    {
      char        buf[256];
      const char *tagname;

      err = tagdb__tagtoname(state->db, tags[i], buf, sizeof(buf));
      if (err)
        goto failure;

      /* sometimes when renaming we'll try to rename it to a single character
       * name which we've already used, in which case we'll clash. cope with
       * that by re-trying. */

      for (;;)
      {
        tagname = randomtagname();

        free(tagnames[i]);

        tagnames[i] = str_dup(tagname);
        if (tagnames[i] == NULL)
        {
          err = error_OOM;
          goto failure;
        }

        printf("renaming '%s' to '%s'", buf, tagnames[i]);

        err = tagdb__rename(state->db, tags[i], tagnames[i]);
        if (err == error_OK)
          break;

        if (err != error_ATOM_NAME_EXISTS)
          goto failure;

        printf("..name clash..");
      }

      err = tagdb__tagtoname(state->db, tags[i], buf, sizeof(buf));
      if (err)
        goto failure;

      assert(strcmp(tagnames[i], buf) == 0);

      printf("..ok\n");
    }

    printf("bash: enumerate\n");

    err = bash_enumerate(state);
    if (err)
      goto failure;

    printf("bash: untag random ids with random tags randomly\n");

    for (i = 0; i < nuntaggings; i++)
    {
      int whichid;
      int whichtag;

      do
        whichid = rnd(nids) - 1;
      while (idnames[whichid] == NULL);

      do
        whichtag = rnd(ntags) - 1;
      while (tagnames[whichtag] == NULL);

      printf("untagging '%s' with %d\n", idnames[whichid], tags[whichtag]);

      err = tagdb__untagid(state->db, idnames[whichid], tags[whichtag]);
      if (err)
        goto failure;
    }

    printf("bash: enumerate\n");

    err = bash_enumerate(state);
    if (err)
      goto failure;

    printf("bash: delete every other tag\n");

    for (i = 0; i < ntags; i += 2)
    {
      printf("removing tag %d ('%s')\n", tags[i], tagnames[i]);

      free(tagnames[i]);
      tagnames[i] = NULL;

      tagdb__remove(state->db, tags[i]);
      tags[i] = -1;
    }

    printf("bash: enumerate\n");

    err = bash_enumerate(state);
    if (err)
      goto failure;

    printf("bash: delete every other id\n");

    for (i = 0; i < nids; i += 2)
    {
      printf("removing id %d '%s'\n", i, idnames[i]);

      tagdb__forget(state->db, idnames[i]);

      free(idnames[i]);
      idnames[i] = NULL;
    }

    printf("bash: enumerate\n");

    err = bash_enumerate(state);
    if (err)
      goto failure;
  }

  for (i = 0; i < nids; i++)
    free(idnames[i]);

  for (i = 0; i < ntags; i++)
    free(tagnames[i]);

  tagdb__close(state->db); /* remember that this calls __commit */

  tagdb__delete(FILENAME);

  return error_OK;


failure:

  return err;
}

/* ----------------------------------------------------------------------- */

typedef error testfn(State *state);

typedef struct
{
  testfn     *fn;
  const char *desc;
}
Test;

int tagdb_test(void)
{
  static const Test tests[] =
  {
    { test_create,
      "create" },
    { test_open,
      "open" },
    { test_add_tags,
      "add tags" },
    { test_rename_tags,
      "rename tags" },
    { test_tag_id,
      "tag id" },
    { test_enumerate_tags,
      "enumerate tags & tag to name" },
    { test_get_tags_for_id,
      "get tags for id" },
    { test_enumerate_ids,
      "enumerate all ids" },
    { test_enumerate_ids_by_tag,
      "enumerate ids by tag" },
    { test_enumerate_ids_by_tags,
      "enumerate ids by tags" },
    { test_commit,
      "commit" },
    { test_tag_remove,
      "remove all tags" },
    { test_get_tags_for_id,
      "get tags for id" },
    { test_forget,
      "forget" },
    { test_get_tags_for_id,
      "get tags for id" },
    { test_close,
      "close" },
    { test_delete,
      "delete" },
    { test_bash,
      "bash" },
  };

  error err;
  State state;
  int   i;

  printf("test: init\n");

  err = tagdb__init();
  if (err)
    goto Failure;

  for (i = 0; i < NELEMS(tests); i++)
  {
    printf("test: %s\n", tests[i].desc);

    err = tests[i].fn(&state);
    if (err)
      goto Failure;
  }

  tagdb__fin();

  return 0;


Failure:

  printf("\n\n*** Error %lx\n", err);

  return 1;
}
