/* --------------------------------------------------------------------------
 *    Name: dcs-quit.c
 * Purpose: Discard/Cancel/Save and Quit dialogues
 * ----------------------------------------------------------------------- */

#include <stdio.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/icon.h"
#include "appengine/base/messages.h"
#include "appengine/types.h"
#include "appengine/wimp/window.h"

#include "appengine/dialogues/dcs-quit.h"

/* ---------------------------------------------------------------------- */

/* Icons */
enum
{
  DCS_D_QUERY    = 0,
  DCS_B_DISCARD  = 1,
  DCS_B_CANCEL   = 2,
  DCS_B_SAVE     = 3,

  QUIT_D_QUERY   = 0, /* keep in sync with DCS_D_QUERY */
  QUIT_B_DISCARD = 1,
  QUIT_B_CANCEL  = 2,
};

/* ---------------------------------------------------------------------- */

typedef struct dcs_t
{
  dialogue_t dialogue; /* base class */
}
dcs_t;

/* ---------------------------------------------------------------------- */

static struct
{
  dialogue_t *dcs_w;
  dialogue_t *quit_w;
}
LOCALS;

/* ----------------------------------------------------------------------- */

static event_wimp_handler    dcs_event_mouse_click;

static event_message_handler dcs_message_menus_deleted;

/* ----------------------------------------------------------------------- */

static result_t dcs_create(const char *name, dialogue_t **new_d)
{
  result_t err;
  dcs_t   *s;

  *new_d = NULL;

  s = calloc(1, sizeof(*s));
  if (s == NULL)
    return result_OOM;

  err = dialogue_construct(&s->dialogue, name, -1, -1);
  if (err)
    goto failure;

  dialogue_set_mouse_click_handler(&s->dialogue, dcs_event_mouse_click);
  dialogue_set_menus_deleted_handler(&s->dialogue, dcs_message_menus_deleted);

  *new_d = &s->dialogue;

  return result_OK;

failure:

  free(s);

  return err;
}

static void dcs_destroy(dialogue_t *d)
{
  dcs_t *s;

  if (d == NULL)
    return;

  s = (dcs_t *) d;

  dialogue_destruct(&s->dialogue);

  free(s);
}

/* ----------------------------------------------------------------------- */

static unsigned int dcs_quit_refcount = 0;

result_t dcs_quit_init(void)
{
  result_t err;

  if (dcs_quit_refcount == 0)
  {
    /* dependencies */

    err = help_init();
    if (err)
      goto failure;

    /* initialise */

    err = dcs_create("dcs",  &LOCALS.dcs_w);
    if (err)
      goto failure;

    err = dcs_create("quit", &LOCALS.quit_w);
    if (err)
      goto failure;
  }

  dcs_quit_refcount++;

  return result_OK;


failure:

  return err;
}

void dcs_quit_fin(void)
{
  if (dcs_quit_refcount == 0)
    return;

  if (--dcs_quit_refcount == 0)
  {
    /* finalise */

    dcs_destroy(LOCALS.quit_w);
    dcs_destroy(LOCALS.dcs_w);

    /* reset handles in case we're reinitialised */

    LOCALS.dcs_w  = NULL;
    LOCALS.quit_w = NULL;

    /* dependencies */

    help_fin();
  }
}

/* ----------------------------------------------------------------------- */

static int dcs_command;

/* ----------------------------------------------------------------------- */

static int query(const char *message, int count, dialogue_t *d)
{
  wimp_w w;
  char   formatted[256];

  w = dialogue_get_window(d);

  sprintf(formatted, message0(message), count);

  icon_set_text(w, DCS_D_QUERY, formatted);

  dialogue_show(d);

  wimp_set_caret_position(w, wimp_ICON_WINDOW, 0, 0, -1, -1);

  dcs_command = -1;

  do
  {
    wimp_block block;

    (void) event_poll(&block);
  }
  while (dcs_command == -1);

  dialogue_hide(d);

  return dcs_command;
}

int dcs_quit_dcs_query(const char *message)
{
  return query(message, 0, LOCALS.dcs_w);
}

int dcs_quit_quit_query(const char *message, int count)
{
  return query(message, count, LOCALS.quit_w);
}

/* ----------------------------------------------------------------------- */

static int dcs_event_mouse_click(wimp_event_no event_no,
                                 wimp_block   *block,
                                 void         *handle)
{
  wimp_pointer *pointer;

  NOT_USED(event_no);
  NOT_USED(handle);

  pointer = &block->pointer;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    switch (pointer->i)
    {
    case DCS_B_DISCARD:
      dcs_command = dcs_quit_DISCARD;
      break;

    case DCS_B_CANCEL:
      dcs_command = dcs_quit_CANCEL;
      break;

    case DCS_B_SAVE:
      dcs_command = dcs_quit_SAVE;
      break;
    }
  }

  return event_HANDLED;
}

static int dcs_message_menus_deleted(wimp_message *message, void *handle)
{
  NOT_USED(message);
  NOT_USED(handle);

  /* If the menu was closed before the user made a choice, then choose
   * Cancel. */

  if (dcs_command == -1)
    dcs_command = dcs_quit_CANCEL;

  return event_HANDLED;
}
