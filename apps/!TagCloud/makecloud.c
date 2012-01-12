/* --------------------------------------------------------------------------
 *    Name: makecloud.c
 * Purpose: Test
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

#include "oslib/os.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/gadgets/tag-cloud.h"
#include "appengine/base/messages.h"
#include "appengine/base/strings.h"
#include "appengine/datastruct/array.h"

#include "makecloud.h"

/* ----------------------------------------------------------------------- */

static struct
{
  tag_cloud *tc;
}
LOCALS;

/* ----------------------------------------------------------------------- */

static int rnd(int mod)
{
  return (rand() % mod) + 1;
}

static const char *randomtagname(void)
{
  static char buf[10 + 1];

  int length;
  int i;

  length = rnd(NELEMS(buf) - 1);

  for (i = 0; i < length; i++)
    buf[i] = 'a' + rnd(26) - 1;

  buf[i] = '\0';

  return buf;
}

/* ----------------------------------------------------------------------- */

static int ntags = 50;
static tag_cloud_tag tags[200];
static unsigned int highlights[(NELEMS(tags) + 31) / 32];
static int indices[NELEMS(tags)];

static void make_tags(void)
{
  const char *name;
  int         i;

  for (i = 0; i < ntags; i++)
  {
    name = randomtagname();

    tags[i].name  = str_dup(name);
    tags[i].count = rnd(100);
    if (tags[i].name == NULL)
      return; // OOM
  }
}

static error set_tags(void)
{
  error err;

  err = tag_cloud_set_tags(LOCALS.tc, tags, ntags);
  if (err)
    return err;

  return error_OK;
}

static error set_highlights(void)
{
  error err;
  int  *p;
  int   i;

  p = indices;
  for (i = 0; i < ntags; i++)
    if (highlights[i >> 5] & (1u << (i & 31)))
      *p++ = i;

  err = tag_cloud_highlight(LOCALS.tc, indices, p - indices);
  if (err)
    return err;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static error add_tag(tag_cloud  *tc,
                     const char *name,
                     int         length,
                     void       *arg)
{
  error err;

  /* add the new tag */

  tags[ntags].name  = str_dup(name);
  tags[ntags].count = 1;
  ntags++;

  /* then call tag_cloud_set_tags */

  err = set_tags();
  if (err)
    return err;

  return error_OK;
}

static error delete_tag(tag_cloud *tc,
                        int        index,
                        void      *arg)
{
  error err;

  /* delete the tag */

  array_delete_element(tags, sizeof(tags[0]), ntags, index);
  ntags--;

  /* then call tag_cloud_set_tags */

  err = set_tags();
  if (err)
    return err;

  return error_OK;
}

static error rename_tag(tag_cloud  *tc,
                        int         index,
                        const char *name,
                        int         length,
                        void       *arg)
{
  error err;

  /* rename the tag */

  free((char *) tags[index].name);
  tags[index].name = str_dup(name);

  /* then call tag_cloud_set_tags */

  err = set_tags();
  if (err)
    return err;

  return error_OK;
}

static error tag(tag_cloud *tc,
                 int        index,
                 void      *arg)
{
  error err;

  err = tag_cloud_add_highlight(LOCALS.tc, index);
  if (err)
    return err;

  highlights[index >> 5] |= 1u << (index & 31);

  return error_OK;
}

static error detag(tag_cloud *tc,
                   int        index,
                   void      *arg)
{
  tag_cloud_remove_highlight(LOCALS.tc, index);

  highlights[index >> 5] &= ~(1u << (index & 31));

  return error_OK;
}

static error tag_file(tag_cloud  *tc,
                      const char *file_name,
                      int         index,
                      void       *arg)
{
  return error_OK;
}

static error detag_file(tag_cloud  *tc,
                        const char *file_name,
                        int         index,
                        void       *arg)
{
  return error_OK;
}

static error event(tag_cloud        *tc,
                   tag_cloud_event  event,
                   void             *arg)
{
  NOT_USED(tc);
  NOT_USED(arg);

  switch (event)
  {
  case tag_cloud_EVENT_COMMIT:
    break;
  }

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static tag_cloud_event keyhandler(wimp_key_no  key_no,
                                   void        *arg)
{
  int op;

  NOT_USED(arg);

  switch (key_no)
  {
  case wimp_KEY_F2:                    return tag_cloud_EVENT_DISPLAY_LIST;
  case wimp_KEY_F3:                    return tag_cloud_EVENT_DISPLAY_CLOUD;

  case wimp_KEY_F2 | wimp_KEY_SHIFT:   return tag_cloud_EVENT_SORT_BY_NAME;
  case wimp_KEY_F3 | wimp_KEY_SHIFT:   return tag_cloud_EVENT_SORT_BY_COUNT;

  case wimp_KEY_F3 | wimp_KEY_CONTROL: return tag_cloud_EVENT_SORT_SELECTED_FIRST;

  case 'R' - 'A' + 1:                  return tag_cloud_EVENT_RENAME;
  case 'K' - 'A' + 1:                  return tag_cloud_EVENT_KILL;
  case 'I' - 'A' + 1:                  return tag_cloud_EVENT_INFO;

  case 'N' - 'A' + 1:                  return tag_cloud_EVENT_NEW;

  case wimp_KEY_F4:                    return tag_cloud_EVENT_COMMIT;

  case wimp_KEY_F2 | wimp_KEY_CONTROL: return tag_cloud_EVENT_CLOSE;
  }

  return tag_cloud_EVENT_UNKNOWN;
}

/* ----------------------------------------------------------------------- */

static int makecloud_refcount = 0;

error makecloud_init(void)
{
  error err;

  if (makecloud_refcount++ == 0)
  {
    tag_cloud_config  conf;
    tag_cloud         *tc = NULL;

    conf.size    = -1; /* use desktop font size */
    conf.leading = (int) (1.4 * 256); /* 8.8 fixed point */
    conf.padding = 32; /* OS units */
    conf.scale   = 100; /* percent */

    tc = tag_cloud_create(0 /* flags */, &conf);
    if (tc == NULL)
    {
      err = error_OOM;
      goto Failure;
    }

    tag_cloud_set_handlers(tc,
                            add_tag,
                            delete_tag,
                            rename_tag,
                            tag,
                            detag,
                            tag_file,
                            detag_file,
                            event,
                            NULL /* opaque */);

    tag_cloud_set_key_handler(tc, keyhandler, NULL /* opaque */);

    LOCALS.tc = tc;
  }

  return error_OK;

Failure:

  return err;
}

void makecloud_fin(void)
{
  if (makecloud_refcount == 0)
    return;

  if (--makecloud_refcount == 0)
  {
    tag_cloud_destroy(LOCALS.tc);
  }
}

/* ----------------------------------------------------------------------- */

void make_cloud(void)
{
  error             err;
  wimp_window_state state;

  state.w = tag_cloud_get_window_handle(LOCALS.tc);
  wimp_get_window_state(&state);

  if (state.flags & wimp_WINDOW_OPEN)
  {
    /* bring to front and gain focus */

    state.next = wimp_TOP;
    wimp_open_window((wimp_open *) &state);
  }
  else
  {
    make_tags();
    set_tags();
    set_highlights();

    tag_cloud_open(LOCALS.tc);
  }

  /* FIXME: This doesn't work - needs investigation. */
  /* wimp_set_caret_position(state.w, wimp_ICON_WINDOW, -1, -1, -1, -1); */

  return;
}
