/* --------------------------------------------------------------------------
 *    Name: name.c
 * Purpose: Name dialogue (inputting a single line of text)
 * Version: $Id: name.c,v 1.8 2009-05-20 21:20:41 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/base/oserror.h"
#include "appengine/wimp/icon.h"

#include "appengine/dialogues/name.h"

/* ----------------------------------------------------------------------- */

/* Icons */

#define ICON_B_CANCEL 0
#define ICON_B_ADD    1
#define ICON_W_NAME   3

/* ----------------------------------------------------------------------- */

typedef struct name_t
{
  dialogue_t             dialogue; /* base class */
  name__ok_handler      *ok_handler;
  void                  *ok_arg;
}
name_t;

/* ----------------------------------------------------------------------- */

static dialogue__fillout  name__default_fillout;

static event_wimp_handler name__event_mouse_click;

/* ----------------------------------------------------------------------- */

dialogue_t *name__create(const char *template)
{
  name_t *n;

  n = calloc(1, sizeof(*n));
  if (n == NULL)
    return NULL;

  dialogue__construct(&n->dialogue, template, ICON_B_ADD, ICON_B_CANCEL);

  dialogue__set_fillout_handler(&n->dialogue,
                                name__default_fillout,
                                n);

  dialogue__set_handlers(&n->dialogue,
                         name__event_mouse_click,
                         NULL,
                         NULL);

  return &n->dialogue;
}

void name__destroy(dialogue_t *d)
{
  name_t *n;

  n = (name_t *) d;

  dialogue__destruct(&n->dialogue);

  free(n);
}

void name__set(dialogue_t *d, const char *name)
{
  icon_set_text(dialogue__get_window(d), ICON_W_NAME, name);
}

/* ----------------------------------------------------------------------- */

/* Default fillout handler. Can be overridden by the client registering their
 * own using dialogue__set_fillout_handler.
 */
static void name__default_fillout(dialogue_t *d, void *arg)
{
  NOT_USED(arg);

  name__set(d, "");
}

static int name__event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer *pointer;
  name_t       *d;

  NOT_USED(event_no);

  pointer = &block->pointer;
  d       = handle;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    osbool close_dialogue = TRUE;

    switch (pointer->i)
    {
    case ICON_B_ADD:
    {
      char buf[64]; /* Careful Now */

      icon_get_text(dialogue__get_window(&d->dialogue), ICON_W_NAME, buf);

      if (buf[0] != '\0')
      {
        if (d->ok_handler)
          d->ok_handler(&d->dialogue, buf, d->ok_arg);
      }
      else
      {
        oserror__report(0, "error.name.missing");
        close_dialogue = FALSE;
      }
    }
      break;

    case ICON_B_CANCEL:
      d->dialogue.fillout(&d->dialogue, d->dialogue.arg);
      break;
    }

    if (close_dialogue)
      if ((pointer->buttons & wimp_CLICK_SELECT) &&
          (pointer->i == ICON_B_ADD || pointer->i == ICON_B_CANCEL))
        dialogue__hide(&d->dialogue);
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

void name__set_ok_handler(dialogue_t       *d,
                          name__ok_handler *handler,
                          void             *arg)
{
  name_t *n = (name_t *) d;

  n->ok_handler = handler;
  n->ok_arg     = arg;
}
