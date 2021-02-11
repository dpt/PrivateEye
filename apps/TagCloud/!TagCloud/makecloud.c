/* --------------------------------------------------------------------------
 *    Name: makecloud.c
 * Purpose: Test
 * ----------------------------------------------------------------------- */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/os.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
#include "appengine/base/strings.h"
#include "appengine/datastruct/array.h"
#include "appengine/gadgets/tag-cloud.h"

#include "makecloud.h"

/* ----------------------------------------------------------------------- */

static struct
{
  tag_cloud *tc;
}
LOCALS;

/* ----------------------------------------------------------------------- */

/* Returns a random integer from 1..mod. */
static int rnd(int mod)
{
  return (rand() % mod) + 1;
}

static const char *randomtagname(size_t *plength)
{
  static const char cns[] = "bbccccdddddddffffggghhhhhhhhhhklllllllmmmmnnnnnnnnnnnnppprrrrrrrrrrsssssssssssttttttttttttttttvvwwwyyy";
  static const char vwl[] = "aaaeeeeeiiiooou";

  static char buf[14 + 1];

  int length;
  int i;

  enum
  {
    Run1,
    Run2,
    Space1,
    Run3,
    Run4,
    OLimit
  }
  ostate;

  enum
  {
    Cons1,
    Vowel1,
    Vowel2,
    Limit
  }
  state;

  length = rnd(NELEMS(buf) - 1);
  if (length < 5)
    length = 5;

  ostate = Run1;
  state = Cons1;

  for (i = 0; i < length; i++)
  {
    int c = 0;

    switch (ostate)
    {
    default:
      switch (state)
      {
      case Cons1:
        c = cns[rnd(NELEMS(cns) - 1) - 1];
        break;

      case Vowel1:
      case Vowel2:
        c = vwl[rnd(NELEMS(vwl) - 1) - 1];
        if (state == Vowel1)
          if (rnd(100) >= 50)
            state++;
        break;
      }

      state++;
      if (state >= Limit)
      {
        state = Cons1;
        ostate++;
      }
      break;

    case Space1:
      c = ' ';
      ostate++;
    }

    buf[i] = (state == Vowel1 && (ostate == Run1 || ostate == Run3)) ? toupper(c) : tolower(c);
  }

  if (isspace(buf[i - 1]))
  {
    i--;
    length--;
  }

  buf[i] = '\0';

  if (plength)
    *plength = length;

  return buf;
}

/* ----------------------------------------------------------------------- */

#define NTAGS 100
static int ntags = NTAGS;
static tag_cloud_tag tags[NTAGS];
static unsigned int highlights[(NTAGS + 31) / 32];
static int indices[NTAGS];

static result_t make_tags(void)
{
  int i;

  for (i = 0; i < ntags; i++)
  {
    const char *name;
    size_t      length;
    char       *newname;

    name = randomtagname(&length);

    // ought to check for dupes here

    newname = malloc(length);
    if (newname == NULL)
      return result_OOM;

    memcpy(newname, name, length);

    tags[i].name   = newname;
    tags[i].length = length;
    tags[i].count  = rnd(ntags);
  }

#ifdef FORTIFY
  Fortify_CheckAllMemory();
#endif

  return result_OK;
}

static result_t set_tags(void)
{
  result_t err;

  err = tag_cloud_set_tags(LOCALS.tc, tags, ntags);
  if (err)
    return err;

#ifdef FORTIFY
  Fortify_CheckAllMemory();
#endif

  return result_OK;
}

static result_t set_highlights(void)
{
  result_t err;
  int  *p;
  int   i;

  p = indices;
  for (i = 0; i < ntags; i++)
    if (highlights[i >> 5] & (1u << (i & 31)))
      *p++ = i;

  err = tag_cloud_highlight(LOCALS.tc, indices, p - indices);
  if (err)
    return err;

#ifdef FORTIFY
  Fortify_CheckAllMemory();
#endif

  return result_OK;
}

/* ----------------------------------------------------------------------- */

static result_t add_tag(tag_cloud  *tc,
                        const char *name,
                        int         length,
                        void       *opaque)
{
  result_t err;
  char *newname;
  int   i;

  /* does it exist already? */

  for (i = 0; i < ntags; i++)
    if (tags[i].length == length && memcmp(tags[i].name, name, length) == 0)
      return result_OK; /* yes - ignore the request */

  /* add the new tag */

  newname = malloc(length);
  if (newname == NULL)
    return result_OOM;

  memcpy(newname, name, length);

  tags[ntags].name   = newname;
  tags[ntags].length = length;
  tags[ntags].count  = 1;
  ntags++;

  /* then call tag_cloud_set_tags */

  err = set_tags();
  if (err)
    return err;

#ifdef FORTIFY
  Fortify_CheckAllMemory();
#endif

  return result_OK;
}

static result_t delete_tag(tag_cloud *tc,
                           int        index,
                           void      *opaque)
{
  result_t err;

  /* delete the tag */

  array_delete_element(tags, sizeof(tags[0]), ntags, index);
  ntags--;

  // need to wipe any highlight

  /* then call tag_cloud_set_tags */

  err = set_tags();
  if (err)
    return err;

#ifdef FORTIFY
  Fortify_CheckAllMemory();
#endif

  return result_OK;
}

static result_t rename_tag(tag_cloud  *tc,
                           int         index,
                           const char *name,
                           int         length,
                           void       *opaque)
{
  result_t err;
  char *newname;

  /* rename the tag */

  if (tags[index].length != length)
  {
    free((char *) tags[index].name);

    newname = malloc(length);
    if (newname == NULL)
      return result_OOM;

    tags[index].length = length;
  }
  else
  {
    newname = (char *) tags[index].name;
  }

  memcpy(newname, name, length);

  tags[index].name = newname;

  /* then call tag_cloud_set_tags */

  err = set_tags();
  if (err)
    return err;

#ifdef FORTIFY
  Fortify_CheckAllMemory();
#endif

  return result_OK;
}

static result_t tag(tag_cloud *tc,
                    int        index,
                    void      *opaque)
{
  result_t err;

  err = tag_cloud_add_highlight(LOCALS.tc, index);
  if (err)
    return err;

  highlights[index >> 5] |= 1u << (index & 31);

#ifdef FORTIFY
  Fortify_CheckAllMemory();
#endif

  return result_OK;
}

static result_t detag(tag_cloud *tc,
                      int        index,
                      void      *opaque)
{
  tag_cloud_remove_highlight(LOCALS.tc, index);

  highlights[index >> 5] &= ~(1u << (index & 31));

#ifdef FORTIFY
  Fortify_CheckAllMemory();
#endif

  return result_OK;
}

static result_t tag_file(tag_cloud  *tc,
                         const char *file_name,
                         int         index,
                         void       *opaque)
{
  return result_OK;
}

static result_t detag_file(tag_cloud  *tc,
                           const char *file_name,
                           int         index,
                           void       *opaque)
{
  return result_OK;
}

static result_t event(tag_cloud       *tc,
                      tag_cloud_event  event,
                      void            *opaque)
{
  NOT_USED(tc);
  NOT_USED(opaque);

  switch (event)
  {
  case tag_cloud_EVENT_COMMIT:
    break;
  }

  return result_OK;
}

/* ----------------------------------------------------------------------- */

static tag_cloud_event keyhandler(wimp_key_no  key_no,
                                  void       *opaque)
{
  int op;

  NOT_USED(opaque);

  switch (key_no)
  {
  case wimp_KEY_F2:                    return tag_cloud_EVENT_DISPLAY_LIST;
  case wimp_KEY_F3:                    return tag_cloud_EVENT_DISPLAY_CLOUD;

  case wimp_KEY_F4:                    return tag_cloud_EVENT_SCALING_OFF;
  case wimp_KEY_F5:                    return tag_cloud_EVENT_SCALING_ON;

  case wimp_KEY_F6:                    return tag_cloud_EVENT_SORT_BY_NAME;
  case wimp_KEY_F7:                    return tag_cloud_EVENT_SORT_BY_COUNT;

  case wimp_KEY_F3 | wimp_KEY_CONTROL: return tag_cloud_EVENT_SORT_SELECTED_FIRST;

  case 'R' - 'A' + 1:                  return tag_cloud_EVENT_RENAME;
  case 'K' - 'A' + 1:                  return tag_cloud_EVENT_KILL;
  case 'I' - 'A' + 1:                  return tag_cloud_EVENT_INFO;

  case 'N' - 'A' + 1:                  return tag_cloud_EVENT_NEW;

  case wimp_KEY_F10:                   return tag_cloud_EVENT_COMMIT;

  case wimp_KEY_F2 | wimp_KEY_CONTROL: return tag_cloud_EVENT_CLOSE;
  }

#ifdef FORTIFY
  Fortify_CheckAllMemory();
#endif

  return tag_cloud_EVENT_UNKNOWN;
}

/* ----------------------------------------------------------------------- */

static unsigned int makecloud_refcount = 0;

result_t makecloud_init(void)
{
  result_t err;

  if (makecloud_refcount == 0)
  {
    tag_cloud_config  conf;
    tag_cloud         *tc = NULL;

    conf.size    = -1; /* use desktop font size */
    conf.leading = (int) (1.4 * 256); /* 8.8 fixed point */
    conf.padding = 16; /* OS units */
    conf.scale   = 100; /* percent */

    tc = tag_cloud_create(0 /* flags */, &conf);
    if (tc == NULL)
    {
      err = result_OOM;
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

  makecloud_refcount++;

#ifdef FORTIFY
  Fortify_CheckAllMemory();
#endif

  return result_OK;


Failure:

  return err;
}

void makecloud_fin(void)
{
#ifdef FORTIFY
  Fortify_CheckAllMemory();
#endif

  if (makecloud_refcount == 0)
    return;

  if (--makecloud_refcount == 0)
  {
    tag_cloud_destroy(LOCALS.tc);
  }
}

/* ----------------------------------------------------------------------- */

result_t make_cloud(void)
{
  result_t          err;
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
    err = make_tags(); // errors not handled
    if (err)
      goto Failure;

    err = set_tags();
    if (err)
      goto Failure;

    err = set_highlights();
    if (err)
      goto Failure;

    tag_cloud_open(LOCALS.tc);
  }

  /* FIXME: This doesn't work - needs investigation. */
  /* wimp_set_caret_position(state.w, wimp_ICON_WINDOW, -1, -1, -1, -1); */

#ifdef FORTIFY
  Fortify_CheckAllMemory();
#endif

  return result_OK;


Failure:

  return err;
}
