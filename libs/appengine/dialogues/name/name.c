/* --------------------------------------------------------------------------
 *    Name: name.c
 * Purpose: Name dialogue (inputting a single line of text)
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
  dialogue_t        dialogue; /* base class */
  name_ok_handler *ok_handler;
  void            *ok_arg;
}
name_t;

/* ----------------------------------------------------------------------- */

static dialogue_fillout name_default_fillout;

static event_wimp_handler name_event_mouse_click;

/* ----------------------------------------------------------------------- */

dialogue_t *name_create(const char *template)
{
  name_t *n;

  n = calloc(1, sizeof(*n));
  if (n == NULL)
    return NULL;

  dialogue_construct(&n->dialogue, template, ICON_B_ADD, ICON_B_CANCEL);

  dialogue_set_fillout_handler(&n->dialogue,
                                name_default_fillout,
                                n);

  dialogue_set_mouse_click_handler(&n->dialogue, name_event_mouse_click);

  return &n->dialogue;
}

void name_destroy(dialogue_t *d)
{
  name_t *n;

  n = (name_t *) d;

  dialogue_destruct(&n->dialogue);

  free(n);
}

void name_set(dialogue_t *d, const char *name)
{
  icon_set_text(dialogue_get_window(d), ICON_W_NAME, name);
}

/* ----------------------------------------------------------------------- */

/* Default fillout handler. Can be overridden by the client registering their
 * own using dialogue_set_fillout_handler.
 */
static void name_default_fillout(dialogue_t *d, void *opaque)
{
  NOT_USED(opaque);

  name_set(d, "");
}

static int name_event_mouse_click(wimp_event_no event_no,
                                  wimp_block   *block,
                                  void         *handle)
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

      icon_get_text(dialogue_get_window(&d->dialogue), ICON_W_NAME, buf);

      if (buf[0] != '\0')
      {
        if (d->ok_handler)
          d->ok_handler(&d->dialogue, buf, d->ok_arg);
      }
      else
      {
        oserror_report(0, "error.name.missing");
        close_dialogue = FALSE;
      }
    }
      break;

    case ICON_B_CANCEL:
      d->dialogue.fillout(&d->dialogue, d->dialogue.opaque);
      break;
    }

    if (close_dialogue)
      if ((pointer->buttons & wimp_CLICK_SELECT) &&
          (pointer->i == ICON_B_ADD || pointer->i == ICON_B_CANCEL))
        dialogue_hide(&d->dialogue);
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

void name_set_ok_handler(dialogue_t      *d,
                         name_ok_handler *handler,
                         void            *opaque)
{
  name_t *n = (name_t *) d;

  n->ok_handler = handler;
  n->ok_arg     = opaque;
}
