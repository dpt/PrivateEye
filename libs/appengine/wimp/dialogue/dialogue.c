/* --------------------------------------------------------------------------
 *    Name: dialogue.c
 * Purpose: Dialogue base class
 * ----------------------------------------------------------------------- */

#include <ctype.h>
#include <stddef.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/wimp/event.h"
#include "appengine/base/errors.h"
#include "appengine/wimp/icon.h"
#include "appengine/wimp/help.h"
#include "appengine/base/os.h"
#include "appengine/wimp/window.h"

#include "appengine/wimp/dialogue.h"

/* ----------------------------------------------------------------------- */

static event_wimp_handler    dialogue_event_mouse_click,
                             dialogue_event_key_pressed;

static event_message_handler dialogue_message_menus_deleted,
                             dialogue_message_menu_warning;

/* ----------------------------------------------------------------------- */

static void dialogue_register_handlers(int reg, dialogue_t *d)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_MOUSE_CLICK,      dialogue_event_mouse_click     },
    { wimp_KEY_PRESSED,      dialogue_event_key_pressed     },
  };

  static const event_message_handler_spec message_handlers[] =
  {
    { message_MENU_WARNING,  dialogue_message_menu_warning  },
    { message_MENUS_DELETED, dialogue_message_menus_deleted },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            d->w, event_ANY_ICON,
                            d);

  event_register_message_group(reg,
                               message_handlers, NELEMS(message_handlers),
                               d->w, event_ANY_ICON,
                               d);
}

/* ----------------------------------------------------------------------- */

static int dialogue_event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer *pointer;
  dialogue_t   *d;

  NOT_USED(event_no);

  pointer = &block->pointer;
  d       = handle;

  /* was the 'Cancel' icon clicked on? */
  if (d->fillout &&
      pointer->i == d->cancel &&
      pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    /* reset the dialogue */

    d->fillout(d, d->arg);
  }
  else if (d->mouse_click)
  {
    /* otherwise pass it down */

    int rc;

    rc = d->mouse_click(event_no, block, d);
    if (rc != event_HANDLED)
      return rc;
  }

  if (d->flags & dialogue_FLAG_KEEP_OPEN)
  {
    d->flags &= ~dialogue_FLAG_KEEP_OPEN;
  }
  else
  {
    if ((pointer->buttons & wimp_CLICK_SELECT) &&
        (pointer->i == d->ok || pointer->i == d->cancel))
      dialogue_hide(d);
  }

  return event_HANDLED;
}

/* Fake a click on a button (or workarea). */
static void fake_click(wimp_w w, wimp_i i, void *handle)
{
  int        shift;
  wimp_block fake;

  shift = inkey(INKEY_SHIFT);

  /* CheckMe: Might want to use the current mouse pos. */
  fake.pointer.pos.x   = 0;
  fake.pointer.pos.y   = 0;
  fake.pointer.buttons = shift ? wimp_CLICK_ADJUST : wimp_CLICK_SELECT;
  fake.pointer.w       = w;
  fake.pointer.i       = i;
  dialogue_event_mouse_click(wimp_MOUSE_CLICK, &fake, handle);
}

static int dialogue_event_key_pressed(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_key   *key;
  dialogue_t *d;
  wimp_key_no c;
  int         i;

  NOT_USED(event_no);

  key = &block->key;
  d   = handle;

  /* Key presses for buttons are case insensitive. */
  c = key->c;
  if (c < 256)
    c = toupper(c);

  /* d->actions contains a list of icons and the keypresses they're
   * associated with. Look up the keypress in that list. */
  for (i = 0; i < d->nactions; i++)
    if (d->actions[i].key_no == c)
      break;

  if (i < d->nactions) /* found */
  {
    fake_click(d->w, d->actions[i].i, handle);
    return event_HANDLED;
  }

  if (d->key_pressed)
    return d->key_pressed(event_no, block, d);

  wimp_process_key(key->c);
  return event_HANDLED;
}

static int dialogue_message_menus_deleted(wimp_message *message, void *handle)
{
  dialogue_t *d;

  NOT_USED(message);

  d = handle;

  /* This delivers an event so that DCS type dialogues can correctly exit
   * their polling loops. */
  if (d->menus_deleted)
    d->menus_deleted(message, handle);

  d->flags &= ~dialogue_FLAG_OPEN;

  return event_HANDLED;
}

static int dialogue_message_menu_warning(wimp_message *message, void *handle)
{
  wimp_message_menu_warning *warning;
  dialogue_t                *d;

  warning = (wimp_message_menu_warning *) &message->data;
  d       = handle;

  /* This code is similar to that in dialogue_show. */

  if (d->fillout)
    d->fillout(d, d->arg); /* get the dialogue filled out */

  wimp_create_sub_menu(warning->sub_menu, warning->pos.x, warning->pos.y);

  d->flags |= dialogue_FLAG_OPEN;

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

/* Adds or replaces the specified icon<->key mapping. */
static void add_action(dialogue_t *d, wimp_i i, wimp_key_no key_no)
{
  int j;

  if (d->nactions == dialogue_MAX_ACTIONS)
    return; /* add no more */

  /* find the specified icon, or the first available free slot */

  for (j = 0; j < d->nactions; j++)
    if (d->actions[j].i == i)
      break;

  d->actions[j].i      = i;
  d->actions[j].key_no = key_no;

  d->nactions = j + 1;
}

/* This handles the icons for DCS style dialogues. */
static void dcs_style_icon(dialogue_t *d, const wimp_icon_state *state)
{
  const wimp_icon_flags FLAGS = wimp_ICON_TEXT     | wimp_ICON_BORDER   |
                                wimp_ICON_HCENTRED | wimp_ICON_VCENTRED |
                                wimp_ICON_FILLED   | wimp_ICON_INDIRECTED;
  char c;

  if ((state->icon.flags & FLAGS) != FLAGS)
    return; /* must be button style */

  if (state->i == d->ok || state->i == d->cancel)
    return;

  c = state->icon.data.indirected_text.text[0];
  if (isalpha(c))
    add_action(d, state->i, toupper(c));
}

/* This handles the icons for writable style dialogues. */
static void fillin_style_icon(dialogue_t *d, const wimp_icon_state *state)
{
  if (state->i == d->ok || state->i == d->cancel)
    return;

  if (state->i <= 10) /* allow up to F11 */
    add_action(d, state->i, state->i + wimp_KEY_F1);
}

static void process_icons(dialogue_t *d, wimp_i *icons,
                     void (*callback)(dialogue_t *, const wimp_icon_state *))
{
  wimp_icon_state state;

  state.w = d->w;

  for (;;)
  {
    wimp_i icn;

    icn = *icons++;
    if (icn == -1)
      break; /* end of list */

    state.i = icn;
    wimp_get_icon_state(&state);

    callback(d, &state);
  }
}

/* We prepare a list of (key,icon) pairs in advance. */
static void assign_key_actions(dialogue_t *d)
{
#define WRITABLE (wimp_BUTTON_WRITABLE << wimp_ICON_BUTTON_TYPE_SHIFT)

  wimp_window_info *defn;
  wimp_i            focus;
  wimp_i            possibles[32]; /* Careful Now */
  void            (*callback)(dialogue_t *d, const wimp_icon_state *state);

  defn = window_get_defn(d->w);

  /* What sort of window is it?
   *
   * Has writable(s) (a 'fill-in' window):
   *   Assign buttons to F keys.
   *   Return activates default action.
   *   Esc does 'Cancel' (assumed to be the icon preceding the default)
   *
   * Has no writables (a 'dcs' window):
   *   Assign letters (A-Z case insens.) to buttons by scanning through them.
   *   Return activates default action.
   */

  focus = icon_find(defn, WRITABLE, WRITABLE);

  callback = (focus < 0) ? dcs_style_icon : fillin_style_icon;

  /* Find all indirected text icons with button type Click or Radio. */
  wimp_which_icon(d->w, possibles, 0x00007101, 0x00003101);

  process_icons(d, possibles, callback);

  free(defn);

  add_action(d, d->ok,     wimp_KEY_RETURN);
  add_action(d, d->cancel, wimp_KEY_ESCAPE);
}

error dialogue_construct(dialogue_t *d,
                          const char *name,
                          wimp_i      ok,
                          wimp_i      cancel)
{
  error err;

  d->w = window_create(name);

  err = help_add_window(d->w, name);
  if (err)
    return err;

  d->ok     = ok;
  d->cancel = cancel;

  assign_key_actions(d);

  dialogue_register_handlers(1, d);

  d->flags = 0;

  return error_OK;
}

void dialogue_destruct(dialogue_t *d)
{
  dialogue_register_handlers(0, d);

  help_remove_window(d->w);
}

/* ----------------------------------------------------------------------- */

void dialogue_set_fillout_handler(dialogue_t        *d,
                                   dialogue_fillout *fillout,
                                   void              *arg)
{
  d->fillout = fillout;
  d->arg     = arg;
}

void dialogue_set_handlers(dialogue_t            *d,
                            event_wimp_handler    *mouse_click,
                            event_wimp_handler    *key_pressed,
                            event_message_handler *menus_deleted)
{
  d->mouse_click   = mouse_click;
  d->key_pressed   = key_pressed;
  d->menus_deleted = menus_deleted;
}

/* ----------------------------------------------------------------------- */

void dialogue_show(dialogue_t *d)
{
  if (d->fillout)
    d->fillout(d, d->arg); /* get the dialogue filled out */

  window_open_as_menu(d->w);

  d->flags |= dialogue_FLAG_OPEN;
}

void dialogue_show_here(dialogue_t *d, int x, int y)
{
  if (d->fillout)
    d->fillout(d, d->arg); /* get the dialogue filled out */

  window_open_as_menu_here(d->w, x, y);

  d->flags |= dialogue_FLAG_OPEN;
}

void dialogue_hide(dialogue_t *d)
{
  if (d->flags & dialogue_FLAG_OPEN)
  {
    wimp_create_menu(wimp_CLOSE_MENU, 0, 0);
    d->flags &= ~dialogue_FLAG_OPEN;
  }
}

/* ----------------------------------------------------------------------- */

wimp_w dialogue_get_window(const dialogue_t *d)
{
  return d->w;
}

void dialogue_keep_open(dialogue_t *d)
{
  d->flags |= dialogue_FLAG_KEEP_OPEN;
}
