/* --------------------------------------------------------------------------
 *    Name: dialogue.h
 * Purpose: Dialogue base class
 * ----------------------------------------------------------------------- */

/* The dialogue class encapsulates those behaviours common to all types of
 * dialogue box.
 *
 * Those are:
 * - Window opened as if a menu
 * - OK and Cancel buttons
 * - F-keys press buttons if writables in window,
 *   single keys press buttons otherwise
 */

#ifndef APPENGINE_DIALOGUE_H
#define APPENGINE_DIALOGUE_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"
#include "appengine/wimp/event.h"

#define T dialogue_t

/* ----------------------------------------------------------------------- */

typedef struct T T;

/* ----------------------------------------------------------------------- */

typedef void (dialogue__fillout)(T *d, void *arg);

/* ----------------------------------------------------------------------- */

#define dialogue__FLAG_OPEN      (1u << 0)
#define dialogue__FLAG_KEEP_OPEN (1u << 1)

typedef unsigned int dialogue__flags;

/* ----------------------------------------------------------------------- */

typedef struct
{
  wimp_i      i;
  wimp_key_no key_no;
}
dialogue__action;

#define dialogue_MAX_ACTIONS 10

/* ----------------------------------------------------------------------- */

/* The dialogue_t structure definition is exposed to allow clients to place
 * it as the first member of their own structure. Its structure members
 * shouldn't be accessed directly.
 */
struct T
{
  dialogue__flags        flags;
  wimp_w                 w;

  wimp_i                 ok;
  wimp_i                 cancel;

  dialogue__action       actions[dialogue_MAX_ACTIONS];
  int                    nactions;

  dialogue__fillout     *fillout; /* called when dialogue about to appear */
  void                  *arg;

  event_wimp_handler    *mouse_click;
  event_wimp_handler    *key_pressed;
  event_message_handler *menus_deleted;
};

/* ----------------------------------------------------------------------- */

/* Creates a dialogue in-place.
 * 'name' is the name of the template.
 * 'ok' and 'cancel' are the icon handles which should close the dialogue
 * when SELECT-clicked, or -1 if none. */
error dialogue__construct(T          *d,
                          const char *name,
                          wimp_i      ok,
                          wimp_i      cancel);
void dialogue__destruct(T *d);

/* The fillout handler gets called before the dialogue is opened, when Cancel
 * is clicked, etc. in order to get all the fields, icons, filled out and set
 * up. It may be NULL. */
void dialogue__set_fillout_handler(T                 *d,
                                   dialogue__fillout *fillout,
                                   void              *arg);

/* These handler routines are called for events which the dialogue code does
 * not know how to handle. They may be NULL. */
void dialogue__set_handlers(T                     *d,
                            event_wimp_handler    *mouse_click,
                            event_wimp_handler    *key_pressed,
                            event_message_handler *menus_deleted);

void dialogue__show(T *d);
void dialogue__show_here(T *d, int x, int y);
void dialogue__hide(T *d);

/* Gets the window handle. */
wimp_w dialogue__get_window(const T *d);

/* In the case of an icon marked as closing the dialogue, this inhibits the
 * close. */
void dialogue__keep_open(T *d);

#undef T

#endif /* APPENGINE_DIALOGUE_H */
